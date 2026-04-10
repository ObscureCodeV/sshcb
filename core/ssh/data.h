#ifndef DATA_H
#define DATA_H

#include <libssh/server.h>
#include <libssh/libssh.h>

//TODO:: add to config
#define CONTEXT_SIZE 2048
#define MAX_CHANNELS 10

struct channel_context {
  char data[CONTEXT_SIZE];
  size_t data_len;
  size_t expected;
  int is_used;
};

struct peer_data {
  ssh_channel channels[MAX_CHANNELS];
  struct channel_context context[MAX_CHANNELS];
  int active_channels; 
};

struct ssh_conn {
  ssh_session session;
  ssh_key key;
  int port;
  struct peer_data data;
};
    
#endif
