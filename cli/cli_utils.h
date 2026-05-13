#ifndef CLI_UTILS_H
#define CLI_UTILS_H

#include "../common/protocol.h"

void print_help(const char *prog);
void parse_command(int argc, char *argv[], ipc_msg_t *msg);
int send_command(ipc_msg_t msg);

#endif
