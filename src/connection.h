#ifndef CONNECTION_H
#define CONNECTION_H

#include <libssh2.h>

struct ConnectedData {
  LIBSSH2_SESSION *session ;
  LIBSSH2_CHANNEL *channel;
  int num_channels;
  char[64] fingerprint;
  libssh2_socket_t sock;
};

int ssh_session_connect(ConnectedData *conn, const char *host, int port);
int ssh_session_accept(ConnectedData *conn, int listen_sock);

#endif
