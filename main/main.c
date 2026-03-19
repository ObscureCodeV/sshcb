#include "../core/ssh/data.h"
#include "../core/ssh/session.h"
#include "../core/ssh/channel.h"
#include "../core/OS/socket_utils.h"
#include "../core/ssh/auth.h"

int main() {
  socket_init();
  socket_cleanup();
  return 0;
}
