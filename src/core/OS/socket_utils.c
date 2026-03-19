#include "socket_utils.h"

int select_timeout(cross_socket sock, fd_set fd_in, fd_set fd_out, int timeout_ms) {
  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  int rc = select(sock, &fd_in, &fd_out, NULL, &tv );

  return rc;
}
