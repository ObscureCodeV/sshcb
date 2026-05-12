#include "../common/socket.h"
#include "../daemon/daemon_utils.h"
#include "../daemon/daemon_wrapper.h"

int main(int argc, char *argv[]) {
  socket_startup();

  if (argc > 1 && strcmp(argv[1], "--daemon") == 0) {
    return daemon_run(daemon_main);
  }

  socket_cleanup();
  return 0;
}
