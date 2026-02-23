#include "ssh/data.h"
#include "ssh/session.h"
#include "ssh/channel.h"
#include "OS/socket_utils.h"

int main() {
  socket_init();
  socket_cleanup();
  return 0;
}
