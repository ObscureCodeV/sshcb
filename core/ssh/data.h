#ifndef DATA_H
#define DATA_H

#include <libssh/server.h>
#include <libssh/libssh.h>

#define ContextSize 2048
#define MaxChannelsNum 10

struct ChannelContext {
  char data[ContextSize];
  size_t data_len;
  size_t expected;
};

struct ssh_conn {
  ssh_session session;
  ssh_channel channels[MaxChannelsNum];
  struct ChannelContext context[MaxChannelsNum];
  ssh_key key;
  int port;
};
    
#endif
