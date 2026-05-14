#include "test.h"
#include <libssh/libssh.h>

int main(int argc, char *argv[]) {
  if(ssh_init() != SSH_OK) {
    return -1;
  }
  test_server(argv[1]);
  return 0;
}
