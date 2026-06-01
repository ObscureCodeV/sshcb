#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "../core/ssh/data.h"

typedef enum {
  CMD_SEND,
  CMD_READ,
  CMD_INIT_CLIENT,
  CMD_INIT_SERVER,
  CMD_SESSION_CLOSE,
  CMD_DAEMON,
  CMD_HELP,
  CMD_NONE
} cmd_type_t;

typedef struct {
  cmd_type_t type;
  uint32_t channel;
  uint32_t data_len;
  char data[CONTEXT_SIZE];
  uint8_t is_success;
  uint8_t is_daemon_response;
} ipc_msg_t;

#endif
