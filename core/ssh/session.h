#ifndef SESSION_H
#define SESSION_H

#include "data.h"

conn_state_t init_server_session(struct ssh_conn *server, const char *listen_ip);
conn_state_t init_user_session(struct ssh_conn *user, const char *host);
conn_state_t ssh_conn_session_close(struct ssh_conn *peer);
conn_state_t start(struct ssh_conn *peer);
conn_state_t stop(struct ssh_conn *peer);

#endif
