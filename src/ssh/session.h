#ifndef SESSION_H
#define SESSION_H

#include "data.h"
#include <libssh2.h>

int ssh_session_connect(ConnectedData *conn, const char *host, int port);
int ssh_session_accept(ConnectedData *conn, int listen_sock);
int ssh_session_close(ConnectedData *conn, const char *description);

#endif
