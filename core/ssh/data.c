#include "data.h"
#include "../logging.h"

#include "stdlib.h"

struct ssh_conn* allocate_buffer() {
  struct ssh_conn *peer = malloc(sizeof(struct ssh_conn));
  if(!peer) {
    log_error(peer->session, "FAILED ALLOCATE SSH_CONN");
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
