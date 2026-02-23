#ifndef DATA_H
#define DATA_H

#include <libssh2.h>

#define ContextSize 2048
#define MaxChannelsNum 10

struct ChannelContext {
  char data[ContextSize];
};

struct ConnectedData {
  LIBSSH2_SESSION *session;
  LIBSSH2_CHANNEL *channels[10];
  struct ChannelContext context[10];
  char fingerprint[64];
  libssh2_socket_t sock;
};

#endif
