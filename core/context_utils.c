#include "context_utils.h"
#include "ssh/channel.h"
#include <string.h>

void write_data(struct channel_pair *pair, const void *buf, const size_t len) {
  mutex_lock(&pair->ctx.mutex);
  pair->ctx.state = STATE_WRITING;
  memcpy(pair->ctx.data, buf, len);
  pair->ctx.data_len = len;
  pair->ctx.state = STATE_DATA_READY;
  mutex_unlock(&pair->ctx.mutex);

  send_data(&pair->channel, &pair->ctx);
}

void read_data(struct channel_pair *pair, char **buf, size_t *len) {
  mutex_lock(&pair->ctx.mutex);
  if(pair->ctx.state == STATE_DATA_READY) {
    memcpy(&buf, pair->ctx.data, pair->ctx.data_len);
    *len = pair->ctx.data_len;
  }
  pair->ctx.state = STATE_RECV_LEN;
  mutex_unlock(&pair->ctx.mutex);
}
