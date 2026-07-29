#ifndef SESSION_H
#define SESSION_H

#include "data.h"

int init_server_session(struct ssh_conn *server, const char *listen_ip);
int init_user_session(struct ssh_conn *user, const char *host);
void ssh_conn_session_close(struct ssh_conn *peer);
void start(struct ssh_conn *peer);
void stop(struct ssh_conn *peer);

#endif
