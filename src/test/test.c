#include "test.h"
#include "./ssh/session.h"

static int test_init_server(struct Conn *server);
static int test_init_client(struct Conn *user);

int test_server() {
  struct Conn *server = malloc(sizeof(struct Conn));
  Conn->port = 1024;
}



