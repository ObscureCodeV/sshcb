#ifndef SESSION_H
#define SESSION_H

#include "data.h"

int init_user_session(struct User *conn, const char *host);
int init_server_session(struct Server *conn);
void server_session_close(struct Server *conn, const char *description);
void user_session_close(struct User *conn, const char *description);

#endif
