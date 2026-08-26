#include "context_utils.h"
#include "ssh/channel.h"
#include "logging.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>

int write_data(struct ssh_conn *conn, int channel_idx, const void *buf, const size_t len) {
  if (conn == NULL) return -1;

  if(channel_idx >= MAX_CHANNELS) return -1;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;
  int retries = 0;

  mutex_lock(&ctx->mutex);

  while(ctx->state != STATE_IDLE && ctx->state != STATE_WRITTEN) {
    cond_timedwait(&ctx->cond, &ctx->mutex, TIME_RETRY);
    if(retries > MAX_RETRIES) {
#ifdef TEST
  log_info(conn->session, "CONTEXT %d WRITE_DATA - STATE TIMEOUT OCCURED", channel_idx);
#endif
      mutex_unlock(&ctx->mutex);
      return -1;
    }
    retries++;
  }

  size_t copy_len = (len < CONTEXT_SIZE) ? len : CONTEXT_SIZE;
  memcpy(ctx->data, buf, copy_len);
  ctx->data_len = copy_len;

  ctx->state = STATE_WRITTEN;
#ifdef TEST
  log_info(conn->session, "CONTEXT %d DATA WRITTEN", channel_idx);
#endif
  mutex_unlock(&ctx->mutex);

  if(conn->data.thread_state == NOT_STARTED)
    return 1;

  return send_data(conn, channel_idx);
}

int read_data(struct ssh_conn *conn, int channel_idx, char *buf) {
  if (conn == NULL) return -1;

  if(channel_idx >= MAX_CHANNELS) return -1;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;
  int retries = 0;

  mutex_lock(&ctx->mutex);

  while(ctx->state != STATE_DATA_READY && ctx->state != STATE_READED && ctx->state != STATE_WRITTEN) {
    cond_timedwait(&ctx->cond, &ctx->mutex, TIME_RETRY);
    if(retries > MAX_RETRIES) {
#ifdef TEST
  log_info(conn->session, "CONTEXT %d  READ_DATA - STATE TIMEOUT OCCURED", channel_idx);
#endif
      mutex_unlock(&ctx->mutex);
      return -1;
    }
    retries++;
  }

  size_t copy_len = ctx->data_len;
  memcpy(buf, ctx->data, ctx->data_len);

  ctx->state = STATE_READED;

#ifdef TEST
  log_info(conn->session, "CONTEXT %d DATA READED", channel_idx);
#endif

  mutex_unlock(&ctx->mutex);

  return copy_len;
}

void clear(struct ssh_conn *conn, int channel_idx) {
  if (conn == NULL) return;

  if(channel_idx >= MAX_CHANNELS) return;
  struct channel_context *ctx = &conn->data.channels_data[channel_idx].ctx;
  ssh_channel *channel = &conn->data.channels_data[channel_idx].channel;

  mutex_lock(&ctx->mutex);
  if (ctx->state == STATE_READED || ctx->state == STATE_DATA_READY) {
    ctx->state = STATE_IDLE;
    cond_signal(&ctx->cond);

#ifdef TEST
  log_info(conn->session, "CONTEXT %d  CLEAR CONTEXT", channel_idx);
#endif

  }
  mutex_unlock(&ctx->mutex);
}

int init_contexts(struct ssh_conn *peer) {
  if(peer == NULL)
    return -1;

  struct channel_context *ctx;
  for(int i = 0; i < MAX_CHANNELS; i++) {
    ctx = &peer->data.channels_data[i].ctx;
    memset(ctx, 0, sizeof(struct channel_context));
    ctx->state = STATE_IDLE;
    mutex_init(&ctx->mutex);
    cond_init(&ctx->cond);

#ifdef TEST
    log_info(peer->session, "CONTEXT %d IS PREINIT", i);
#endif
   }

  return 0;
}
