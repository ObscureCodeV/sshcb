#include "context_utils.h"
#include "ssh/channel.h"
#include <string.h>
#include <stdlib.h>

void write_data(struct channel_pair *pair, const void *buf, const size_t len) {
  mutex_lock(&pair->ctx.mutex);

  while(pair->ctx.state != STATE_IDLE && pair->ctx.state != STATE_DATA_READY) {
    cond_wait(&pair->ctx.cond, &pair->ctx.mutex);
  }

  size_t copy_len = (len < CONTEXT_SIZE) ? len : CONTEXT_SIZE;
  memcpy(pair->ctx.data, buf, copy_len);
  pair->ctx.data_len = copy_len;
  pair->ctx.state = STATE_DATA_READY;

  mutex_unlock(&pair->ctx.mutex);

  send_data(&pair->channel, &pair->ctx);

  mutex_lock(&pair->ctx.mutex);
  pair->ctx.state = STATE_IDLE;
  cond_broadcast(&pair->ctx.cond);
  mutex_unlock(&pair->ctx.mutex);
}

void read_data(struct channel_pair *pair, char **buf, size_t *len) {
  mutex_lock(&pair->ctx.mutex);

  while(pair->ctx.state != STATE_DATA_READY) {
    cond_wait(&pair->ctx.cond, &pair->ctx.mutex);
  }

  *len = pair->ctx.data_len;
  *buf = malloc(*len);

  memcpy(*buf, pair->ctx.data, *len);

  pair->ctx.state = STATE_IDLE;
  cond_broadcast(&pair->ctx.cond);
  mutex_unlock(&pair->ctx.mutex);
}
