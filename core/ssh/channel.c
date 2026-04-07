#include "data.h"
#include "channel.h"
#include "../logging.h"
#include "../config.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int set_callback_channel(ssh_channel *channel, struct ChannelContext *data);
static int recv_data(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata);
static void channel_timeout(ssh_channel channel);


void close_channels(struct ssh_conn *peer) {
  for(int i = 0; i < MaxChannelsNum; i++) {
    if(peer->channels[i] != NULL) {
      ssh_channel_free(peer->channels[i]);
    }
  }
}

int init_user_channels(struct ssh_conn *peer, const char *host) {
  int rc;
  const struct sshcb_config *cfg = NULL;
  const char *error_message;
  int i = 0;

  cfg = sshcb_get_config();
  if(cfg == NULL) {
    error_message = "Failed get sshcb_config";
    goto failure_init_channels;
  }
  
  for(; i < MaxChannelsNum; i++) {
    peer->channels[i] = ssh_channel_new(peer->session);
    if(peer->channels[i] == NULL) {
      peer->context[i].expected = 0;
      peer->context[i].data_len = 0;
      goto failure_init_channels;
    }
    rc = ssh_channel_open_forward(peer->channels[i], host, cfg->server_port, NULL, 0);
    if(rc != SSH_OK) goto failure_init_channels;

    rc = set_callback_channel(&peer->channels[i], &peer->context[i]);
    if(rc != SSH_OK) goto failure_init_channels;
  }

  ssh_set_blocking(peer->session, 0);

  return SSH_OK;

failure_init_channels:
  if(error_message == NULL) error_message = ssh_get_error(peer->channels[i]);
  log_error(peer->session, "Failed init channels");
  close_channels(peer);
  return SSH_ERROR;
}

int init_server_channels(struct ssh_conn *peer) {
  int rc;
  const struct sshcb_config *cfg = NULL;
  const char *error_message;

  cfg = sshcb_get_config();
  if(cfg == NULL) {
    error_message = "Failed get sshcb_config";
    goto failure_init_channels;
  }

  rc = ssh_channel_listen_forward(peer->session, cfg->bind_address, cfg->server_port, NULL);
  if(rc != SSH_OK) goto failure_init_channels;

  for(int i = 0; i < MaxChannelsNum; i++) {
  //TODO:: timeout_ms into config
    peer->channels[i] = ssh_channel_accept_forward(peer->session, 500, NULL);
    if(peer->channels[i] == NULL) {
      peer->context[i].expected = 0;
      peer->context[i].data_len = 0;
      goto failure_init_channels;
    }
    rc = set_callback_channel(&peer->channels[i], &peer->context[i]);
    if(rc != SSH_OK) goto failure_init_channels;
  }

  ssh_set_blocking(peer->session, 0);

  return SSH_OK;

failure_init_channels:
  if(error_message == NULL) error_message = ssh_get_error(peer->session);
  log_error(peer->session, "Failed init channels");
  close_channels(peer);
  return SSH_ERROR;
}

static int set_callback_channel(ssh_channel *channel, struct ChannelContext *data) {
  int rc;
  struct ssh_channel_callbacks_struct cb = {
    .userdata = data,
    .channel_data_function = recv_data
  };
  ssh_callbacks_init(&cb);
  rc = ssh_set_channel_callbacks(*channel, &cb);
  return rc;
}

static int recv_data(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata) {
  struct ChannelContext *ctx = userdata;

  if(ctx->data_len == ctx->expected) {
    ctx->expected = 0;
    ctx->data_len = 0;
    memcpy(&ctx->expected, data, len);
    if(ctx->expected > ContextSize)
      ctx->expected = ContextSize;
    return len;
  }

  if(ctx->data_len < ctx->expected) {
    memcpy(ctx->data + ctx->data_len, data, len);
    ctx->data_len += len;
  }

  return len;
}

int send_data(ssh_channel *channel, struct ChannelContext *context) {
  int bytes_written;
  int total_written = 0;
  int buffer_size = context->data_len;

 while(1) {
    bytes_written = ssh_channel_write(*channel, context->data + total_written, buffer_size - total_written);
    if (bytes_written > 0) {
      total_written += bytes_written;
      if(total_written >= buffer_size) break;
    }
    else if(bytes_written == SSH_AGAIN) {
      channel_timeout(*channel);
      continue;
    }
    else {
      break;
    }
  }
  ssh_channel_send_eof(*channel);
  return total_written;
}

static void channel_timeout(ssh_channel channel) {
  ssh_session session = ssh_channel_get_session(channel);
  ssh_event event = ssh_event_new();
  ssh_event_add_session(event, session);
  ssh_event_dopoll(event, 100);
  ssh_event_free(event);
}

int check_recv_success(const struct ChannelContext *context) {
  return context->expected == context->data_len;
}
