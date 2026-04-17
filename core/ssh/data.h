#ifndef DATA_H
#define DATA_H

#include "../OS/threads.h"
#include "../OS/fd_utils.h"
#include <libssh/server.h>
#include <libssh/libssh.h>

//TODO:: add to config
#define CONTEXT_SIZE 2048
#define MAX_CHANNELS 10

enum channel_state {
  STATE_RECV_LEN,
  STATE_RECV_DATA,
  STATE_DATA_READY,
  STATE_SENDING,
  STATE_READING,
  STATE_WRITING,
  STATE_CLOSED
};

struct channel_context {
  enum channel_state state;
  mutex_t mutex;

  char data[CONTEXT_SIZE];
  size_t data_len;
  size_t expected;

  size_t len_received;
  uint8_t len_buff[4];
};

struct channel_pair {
  ssh_channel channel;
  struct channel_context ctx;
};

struct peer_data {
  struct channel_pair channels_data[MAX_CHANNELS];
  int active_channels;
  mutex_t mutex;
};

struct ssh_conn {
  ssh_session session;
  ssh_key key;
  int port;
  struct peer_data data;
};
    
#endif
