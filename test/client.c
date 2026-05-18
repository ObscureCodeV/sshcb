#include "test.h"
#include <libssh/libssh.h>

int main(int argc, char *argv[]) {
  ssh_init();
  test_client(argv[1]);
  ssh_finalize();
  return 0;
}
