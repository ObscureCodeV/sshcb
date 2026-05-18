#include "context_utils.h"
#include "ssh/channel.h"
#include "logging.h"
#include <string.h>
#include <stdlib.h>

static int session_is_running(struct ssh_conn *conn);

void write_data(struct ssh_conn *conn, int channel_idx, const void *buf, const size_t len) {
  if (conn == NULL) return;

  if(channel_idx >= MAX_CHANNELS) return;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;

  if(!session_is_running(conn)) return;

  mutex_lock(&ctx->mutex);


  while(ctx->state != STATE_IDLE
      && ctx->state != STATE_READED) {
    cond_timedwait(&ctx->cond, &ctx->mutex, 500);
  }

  size_t copy_len = (len < CONTEXT_SIZE) ? len : CONTEXT_SIZE;
  memcpy(ctx->data, buf, copy_len);
  ctx->data_len = copy_len;
  ctx->state = STATE_WRITTEN;
  mutex_unlock(&ctx->mutex);

  send_data(channel, ctx);
}

size_t read_data(struct ssh_conn *conn, int channel_idx, char *buf) {
  if (conn == NULL) return 0;

  if(channel_idx >= MAX_CHANNELS) return 0;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;

  if(!session_is_running(conn)) return 0;

  mutex_lock(&ctx->mutex);

  while(ctx->state != STATE_DATA_READY && ctx->state != STATE_READED) {
    cond_timedwait(&ctx->cond, &ctx->mutex, 500);
  }

  size_t copy_len = ctx->data_len;

  memcpy(buf, ctx->data, ctx->data_len);

  ctx->state = STATE_READED;
  cond_broadcast(&ctx->cond);
  mutex_unlock(&ctx->mutex);

  return copy_len;
}

void clear_readed(struct ssh_conn *conn, int channel_idx) {
  if (conn == NULL) return;

  if(channel_idx >= MAX_CHANNELS) return;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;

  if(!session_is_running(conn)) return;

  mutex_lock(&ctx->mutex);
  if (ctx->state == STATE_READED) {
    ctx->state = STATE_IDLE;
    cond_signal(&ctx->cond);
  }
  mutex_unlock(&ctx->mutex);
}

static int session_is_running(struct ssh_conn *conn) {
  int running;
  mutex_lock(&conn->data.mutex);
  while(conn->data.thread_state == IS_IDLE || conn->data.thread_state == IS_STOPPING) {
    cond_timedwait(&conn->data.cond, &conn->data.mutex, 5000);
  }
  running = (conn->data.thread_state == IS_RUNNING);
  mutex_unlock(&conn->data.mutex);
  return running;
}
