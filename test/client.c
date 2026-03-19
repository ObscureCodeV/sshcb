#include "test.h"
#include "../core/OS/socket_utils.h"
#include <libssh/libssh.h>

int main() {
  socket_init();
  if(ssh_init() != SSH_OK) {
    return -1;
  }
  test_client();
  socket_cleanup();
  return 0;
}
