#include "context_utils.h"
#include "ssh/channel.h"
#include "logging.h"
#include <string.h>
#include <stdlib.h>

void write_data(struct ssh_conn *conn, int channel_idx, const void *buf, const size_t len) {
  if(channel_idx >= MAX_CHANNELS) return;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;

  mutex_lock(&ctx->mutex);
  if(conn->data.thread_state != IS_RUNNING) {
    log_error(conn->session, "write data failed, session not running");
    mutex_unlock(&ctx->mutex);
    return;
  }

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
  if(channel_idx >= MAX_CHANNELS) return 0;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;
  size_t copy_len = ctx->data_len;

  mutex_lock(&ctx->mutex);
  if(conn->data.thread_state != IS_RUNNING) {
    log_error(conn->session, "read data failed, session not running");
    mutex_unlock(&ctx->mutex);
    return 0;
  }

  while(ctx->state != STATE_DATA_READY && ctx->state != STATE_READED) {
    cond_timedwait(&ctx->cond, &ctx->mutex, 500);
  }

  memcpy(buf, ctx->data, ctx->data_len);

  ctx->state = STATE_READED;
  cond_broadcast(&ctx->cond);
  mutex_unlock(&ctx->mutex);

  return copy_len;
}

void clear_readed(struct ssh_conn *conn, int channel_idx) {

  if(channel_idx >= MAX_CHANNELS) return;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;

  mutex_lock(&ctx->mutex);
  if(conn->data.thread_state != IS_RUNNING) {
    log_error(conn->session, "idle channel failed, session not running");
    mutex_unlock(&ctx->mutex);
    return;
  }

  if (ctx->state == STATE_READED) {
    ctx->state = STATE_IDLE;
    cond_signal(&ctx->cond);
  }
  mutex_unlock(&ctx->mutex);
}
