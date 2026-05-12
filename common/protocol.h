#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "../core/ssh/data.h"

typedef enum {
  CMD_SEND,
  CMD_READ,
  CMD_CLEAR,
  CMD_INIT_CLIENT,
  CMD_INIT_SERVER,
  CMD_CLOSE
} cmd_type_t;

typedef struct {
  cmd_type_t type;
  uint32_t channel;
  uint32_t data_len;
  char data[CONTEXT_SIZE];
} ipc_msg_t;

#endif
