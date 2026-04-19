#include "test.h"
#include <libssh/libssh.h>

int main() {
  if(ssh_init() != SSH_OK) {
    return -1;
  }
  test_client();
  return 0;
}
