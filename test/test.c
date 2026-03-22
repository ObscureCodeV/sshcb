#include "test.h"
#include "../core/ssh/session.h"
#include "../core/ssh/key_utils.h"
#include "stdio.h"
#include "stdlib.h"

static int conf_ssh_conn(struct ssh_conn *conn, int port, char *pubkey_file);
static void out_msg(char *msg);

int test_server() {
  struct ssh_conn *server = NULL;
  
  if(conf_ssh_conn(server, 1024, server_pubkey_file) < 0) {
    return -1;
  }

  if(init_server_session(server) < 0) {
    return -1;
  }

  fprintf(stdout, "%s/n", "init server success\0");

  ssh_conn_session_close(server, "end test");

  return 0;
}

int test_client() {
  struct ssh_conn *client = NULL;

  if(conf_ssh_conn(client, 1024, user_pubkey_file) < 0) {
    return -1;
  }

  if(init_user_session(client, "127.0.0.1") < 0) {
    return -1;
  }

  fprintf(stdout, "%s\n", "init client success");

  ssh_conn_session_close(client, "end test");

  return 0;
}

static int conf_ssh_conn(struct ssh_conn *conn, int port, char *pubkey_file) {
  conn = malloc(sizeof(struct ssh_conn));
  conn->port;
  if(ssh_pki_import_pubkey_file(pubkey_file, &conn->key) != SSH_OK) {
    out_msg("bad import pubkey");
    return -1; 
  }
}

static void out_msg(char *msg) {
  fprintf(stdout, "%s\n", msg);
}
