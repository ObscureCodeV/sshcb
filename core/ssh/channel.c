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
static void on_channel_close(ssh_session session, ssh_channel channel, void *userdata);


void close_channels(struct ssh_conn *peer) {
  for(int idx = 0; idx < MAX_CHANNELS; idx++, peer->data.active_channels--) {
    if(peer->data.channels[idx] != NULL) {
      ssh_channel_close(peer->data.channels[idx]);
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
 
  for(; idx < MAX_CHANNELS; idx++, peer->data.active_channels++) {

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
  close_channels(peer);
  return SSH_ERROR;
}


static int set_callback_channel(ssh_channel *channel, struct channel_context *data) {
  int rc;
  struct ssh_channel_callbacks_struct cb = {
    .userdata = data,
    .channel_data_function = recv_data,
    .channel_close_function = on_channel_close
  };
  ssh_callbacks_init(&cb);
  rc = ssh_set_channel_callbacks(*channel, &cb);
  return rc;
}

static int recv_data(ssh_session session, ssh_channel channel, void *data, uint32_t len, int is_stderr, void *userdata) {
  struct channel_context *ctx = userdata;
  uint8_t *bytes = (uint8_t*)data;
  uint32_t remaining = len;


  mutex_lock(&ctx->mutex);
  if(ctx->state == STATE_READING ||
    ctx->state == STATE_SENDING  ||
    ctx->state == STATE_WRITING) {
    mutex_unlock(&ctx->mutex);
    return 0;
  }

  size_t need, to_copy;

  while(remaining > 0) {
    switch(ctx->state) {
      case STATE_RECV_LEN:
        //INFO:: receive 4 bytes for len of message data
        need = 4 - ctx->len_received;
        to_copy = (remaining < need) ? remaining : need;
        memcpy(ctx->len_buff + ctx->len_received, bytes, to_copy);

        ctx->len_received += to_copy;
        bytes += to_copy;
        remaining -= to_copy;

        if(ctx->len_received == 4) {
          ctx->data_len = 0;
          ctx->state = STATE_RECV_DATA;

          ctx->expected = ntohl(*(uint32_t*)ctx->len_buff);
          if (ctx->expected > CONTEXT_SIZE) {
            ctx->expected = CONTEXT_SIZE;
          }
        }
        break;

      case STATE_RECV_DATA:
        need = ctx->expected - ctx->data_len;
        to_copy = (remaining < need) ? remaining : need;
        memcpy(ctx->data + ctx->data_len, bytes, to_copy);

        ctx->data_len += to_copy;
        bytes += to_copy;
        remaining -= to_copy;

        if(ctx->data_len == ctx->expected) {
          ctx->state = STATE_DATA_READY;
          mutex_unlock(&ctx->mutex);
          return len;
        }
        break;

      default:
        mutex_unlock(&ctx->mutex);
        return 0;      
    }
  }

  mutex_unlock(&ctx->mutex);
  return len;
}

int send_data(ssh_channel *channel, struct channel_context *ctx) {

  mutex_lock(&ctx->mutex);
  if(ctx->state != STATE_DATA_READY) {
    mutex_unlock(&ctx->mutex);
    return 0;
  }

  ctx->state = STATE_SENDING;

  size_t data_len = ctx->data_len;
  char temp[data_len];
  memcpy(temp, ctx->data, data_len);

  mutex_unlock(&ctx->mutex);

  uint32_t net_len = htonl(data_len);
  ssh_channel_write(*channel, &net_len, sizeof(net_len));

  int bytes_written;
  int total_written = 0;
  
  while(total_written < data_len) {
    bytes_written = ssh_channel_write(*channel, temp + total_written, data_len - total_written);
    if (bytes_written > 0) {
      total_written += bytes_written;
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

  mutex_lock(&ctx->mutex); 
  ctx->state = STATE_DATA_READY;
  mutex_unlock(&ctx->mutex);
  return total_written;
}

static void channel_timeout(ssh_channel channel) {
  ssh_session session = ssh_channel_get_session(channel);
  ssh_event event = ssh_event_new();
  ssh_event_add_session(event, session);
  ssh_event_dopoll(event, 100);
  ssh_event_free(event);
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

  log_info(session, "%s\ %i", "open server channel idx:", current);
  server_data->active_channels++;

  int rc = set_callback_channel(&server_data->channels[current], &server_data->context[current]);

  if(rc != SSH_OK) {
    server_data->active_channels--;
    on_channel_close(session, server_data->channels[current], server_data);
    log_error(session, "%s\t%c", "failed set channel callback");
    return NULL;
  }

  return server_data->channels[current];
}

static void on_channel_close(ssh_session session, ssh_channel channel, void *userdata) {
  struct channel_context *ctx = userdata;
  if(!ctx) return;

  mutex_lock(&ctx->mutex);
  ctx->state = STATE_CLOSED;
  mutex_unlock(&ctx->mutex);

  mutex_destroy(&ctx->mutex);

  ssh_channel_close(channel);
}

static void init_channel_context(struct channel_context *ctx) {
  ctx->data_len = 0;
  ctx->expected = 0;
  ctx->len_received = 0;
  ctx->state = STATE_RECV_LEN;
  memset(ctx->data, 0, sizeof(ctx->data));
  mutex_init(&ctx->mutex);
}
