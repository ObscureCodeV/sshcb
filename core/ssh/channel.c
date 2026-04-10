#include "data.h"
#include "channel.h"
#include "../logging.h"
#include "../config.h"
#include "../OS/threads.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int set_callback_channel(ssh_channel *channel, struct channel_context *data);
static int recv_data(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata);
static void channel_timeout(ssh_channel channel);
static void init_channel_context(struct channel_context *ctx);


void close_user_channels(struct ssh_conn *peer) {
  for(int idx = 0; idx < MAX_CHANNELS; idx++) {
    if(peer->data.channels[idx] != NULL) {
      ssh_channel_free(peer->data.channels[idx]);
    }
  }
}

int init_user_channels(struct ssh_conn *peer) {
  int rc;
  const char *error_message = NULL;
  int idx = 0;
  ssh_channel *channel;
  struct channel_context *context;
 
  for(; idx < MAX_CHANNELS; idx++) {

    channel = &peer->data.channels[idx];
    context = &peer->data.context[idx];

    *channel = ssh_channel_new(peer->session);
    if(*channel == NULL) {
      goto failure_init_channels;
    }
    
    init_channel_context(context);

    rc = ssh_channel_open_session(peer->data.channels[idx]);
    if(rc == SSH_ERROR) goto failure_init_channels;
    else if(rc == SSH_AGAIN) {
      while (ssh_channel_is_open(*channel) == 0 && !ssh_channel_is_eof(*channel)) {
        channel_timeout(*channel);
      }
    }

    rc = set_callback_channel(&peer->data.channels[idx], &peer->data.context[idx]);
    if(rc != SSH_OK) goto failure_init_channels;
  }

  return SSH_OK;

failure_init_channels:
  if(error_message == NULL) error_message = ssh_get_error(peer->session);
  log_error(peer->session, "Failed init channels");
  log_error(peer->session, error_message);
  close_user_channels(peer);
  return SSH_ERROR;
}


static int set_callback_channel(ssh_channel *channel, struct channel_context *data) {
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
  struct channel_context *ctx = userdata;

  switch(ctx->state) {
    case STATE_RECV_LEN:
//INFO:: send, recv - started by user
    case STATE_DATA_READY:
      mutex_lock(&ctx->mutex);
      memcpy(&ctx->expected, data, len);
      if(ctx->expected > CONTEXT_SIZE)
        ctx->expected = CONTEXT_SIZE;
      ctx->state = STATE_RECV_DATA;
      mutex_unlock(&ctx->mutex);
      break;

    case STATE_RECV_DATA:
      memcpy(ctx->data + ctx->data_len, data, len);
      ctx->data_len += len;
      mutex_lock(&ctx->mutex);
      if(data_filled(ctx))
        ctx->state = STATE_DATA_READY;
      mutex_unlock(&ctx->mutex);
      break;

    case STATE_SENDING:
      return 0;     
  }

  return len;
}

int send_data(ssh_channel *channel, struct channel_context *channel_data) {
 
  if(channel_data->state != STATE_DATA_READY)
   return 0;

  mutex_lock(&channel_data->mutex);
  channel_data->state = STATE_SENDING;

  int bytes_written;
  int total_written = 0;
  int buffer_size = channel_data->data_len;
  
  while(1) {
    bytes_written = ssh_channel_write(*channel, channel_data->data + total_written, buffer_size - total_written);
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
//INFO:: data can be reused
  channel_data->state = STATE_DATA_READY;
  mutex_unlock(&channel_data->mutex);
  return total_written;
}

static void channel_timeout(ssh_channel channel) {
  ssh_session session = ssh_channel_get_session(channel);
  ssh_event event = ssh_event_new();
  ssh_event_add_session(event, session);
  ssh_event_dopoll(event, 100);
  ssh_event_free(event);
}

int data_filled(const struct channel_context *data) {
  fprintf(stdout, "%s\n", "HERE");
  if(data == NULL) return 0;
  if(data->data_len == 0) return 0;
  fprintf(stdout, "%s\n", "HERE END");
  return data->expected == data->data_len;
}

ssh_channel server_channel_open(ssh_session session, void *userdata) {
  struct peer_data *server_data = userdata;

  if(server_data->active_channels == MAX_CHANNELS)
    return NULL;

  int current = server_data->active_channels;
  server_data->channels[current] = ssh_channel_new(session);

  if(server_data->channels[current] == NULL) {
    log_error(session, "failed open channel");
    return NULL;
  }

  init_channel_context(&server_data->context[current]);

  log_info(session, "%s\t%c", "open server channel idx:", current);
  server_data->active_channels++;

  int rc = set_callback_channel(&server_data->channels[current], &server_data->context[current]);

  if(rc != SSH_OK) {
    server_channel_close(session, server_data->channels[current], server_data);
    log_error(session, "%s\t%c", "failed set channel callback");
    return NULL;
  }

  return server_data->channels[current];
}

void server_channel_close(ssh_session session, ssh_channel channel, void *userdata) {
  //INFO:: closed channel - it's last
  if(channel == NULL) return;
  //DEBUG
  log_info(session, "try to close not open channel");
  ssh_channel_close(channel);
  struct peer_data *server_data = userdata;
  server_data->active_channels--;
  mutex_destroy(&server_data->context[server_data->active_channels].mutex);
  log_info(session, "close channel");
}

static void init_channel_context(struct channel_context *ctx) {
  ctx->data_len = 0;
  ctx->expected = 0;
  ctx->sent_bytes = 0;
  ctx->state = STATE_RECV_LEN;
  memset(ctx->data, 0, sizeof(ctx->data));
  mutex_init(&ctx->mutex);
}
