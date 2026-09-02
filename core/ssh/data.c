#include "data.h"
#include "session.h"
#include "../logging.h"

#include "stdlib.h"

struct ssh_conn* allocate_buffer() {
  struct ssh_conn *peer = malloc(sizeof(struct ssh_conn));
  if(!peer) {
    log_error(NULL, "FAILED ALLOCATE SSH_CONN");
    return NULL;
  }

#ifdef TEST
  log_info(peer->session, "Allocated ssh_conn");
#endif

  peer->data.thread_state = NOT_STARTED;

#ifdef TEST
  log_info(peer->session, "SESSION NOT STARTED");
#endif

  return peer;
}

void deallocate_buffer(struct ssh_conn *peer) {
  if(peer->data.thread_state != NOT_STARTED && peer->data.thread_state != IS_STOPPED) {
    stop(peer);
    ssh_conn_session_close(peer);
  }

  struct channel_context *ctx;
  for(int idx = 0; idx < MAX_CHANNELS; idx++) {
    ctx = &peer->data.channels_data[idx].ctx;
    mutex_destroy(&ctx->mutex);
    cond_destroy(&ctx->cond);
  }
  free(ctx);
}
