#ifndef daemon_utils_h
#define daemon_utils_h

#include "../common/protocol.h"

int daemon_main(void);
void handle_request(struct ssh_conn *conn, ipc_msg_t *packet);

#endif
