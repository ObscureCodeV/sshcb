#include "../common/socket.h"
#include "../cli/cli_utils.h"

int main(int argc, char *argv[]) {
  socket_startup();

  ipc_msg_t msg;

  parse_command(argc, argv, &msg);
  send_command(msg); 

  socket_cleanup();
  return 0;
}
