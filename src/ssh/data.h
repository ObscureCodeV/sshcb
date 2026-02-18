#ifndef DATA_H
#define DATA_H

#include <libssh2.h>
#include <sys/socket.h> 

struct ConnectedData {
  LIBSSH2_SESSION *session;
  LIBSSH2_CHANNEL *channel;
  int num_channels;
  char fingerprint[64];
  libssh2_socket_t sock;
};

#endif
