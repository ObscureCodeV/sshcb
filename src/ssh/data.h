#ifndef DATA_H
#define DATA_H

#include <libssh/server.h>
#include <libssh/libssh.h>

#define ContextSize 2048
#define MaxChannelsNum 10

struct ChannelContext {
  char data[ContextSize];
  int data_len;
};

struct User {
  ssh_session session;
  ssh_channel channels[MaxChannelsNum];
  struct ChannelContext context[MaxChannelsNum];
  ssh_key key;
  int port;
 };

struct Server {
  ssh_session session;
  ssh_channel channels[MaxChannelsNum];
  struct ChannelContext context[MaxChannelsNum];
  ssh_bind bind;
  ssh_key key;
  int port;
};
    
#endif
