#include "context_utils.h"
#include "ssh/channel.h"
#include <string.h>
#include <stdlib.h>

void write_data(struct channel_pair *pair, const void *buf, const size_t len) {
  mutex_lock(&pair->ctx.mutex);

  while(pair->ctx.state != STATE_IDLE
      && pair->ctx.state != STATE_READED) {
    cond_wait(&pair->ctx.cond, &pair->ctx.mutex);
  }

  size_t copy_len = (len < CONTEXT_SIZE) ? len : CONTEXT_SIZE;
  memcpy(pair->ctx.data, buf, copy_len);
  pair->ctx.data_len = copy_len;
  pair->ctx.state = STATE_WRITTEN;
  mutex_unlock(&pair->ctx.mutex);

  send_data(&pair->channel, &pair->ctx);
}

size_t read_data(struct channel_pair *pair, char *buf) {
  size_t copy_len = pair->ctx.data_len;
  mutex_lock(&pair->ctx.mutex);

  while(pair->ctx.state != STATE_DATA_READY && pair->ctx.state != STATE_READED) {
    cond_wait(&pair->ctx.cond, &pair->ctx.mutex);
  }

  memcpy(buf, pair->ctx.data, pair->ctx.data_len);

  pair->ctx.state = STATE_READED;
  cond_broadcast(&pair->ctx.cond);
  mutex_unlock(&pair->ctx.mutex);

  return copy_len;
}

void clear_readed(struct channel_pair *pair) {
  struct channel_context *ctx = &pair->ctx;
  mutex_lock(&ctx->mutex);
  if (ctx->state == STATE_READED) {
    ctx->state = STATE_IDLE;
    cond_signal(&ctx->cond);
  }
  mutex_unlock(&ctx->mutex);
}
