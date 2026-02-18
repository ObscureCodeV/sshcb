#ifndef SESSION_H
#define SESSION_H

#include "data.h"

int ssh_session_connect(struct ConnectedData *conn, const char *host, int port);
int ssh_session_accept(struct ConnectedData *conn, int listen_sock);
int ssh_session_close(struct ConnectedData *conn, const char *description);

#endif
