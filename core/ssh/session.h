#ifndef SESSION_H
#define SESSION_H

#include "data.h"

struct ssh_conn* init_user_session(const char *host);
struct ssh_conn* init_server_session();
void ssh_conn_session_close(struct ssh_conn *peer);
void *session_thread(void *arg);

#endif
