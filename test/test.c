#include "test.h"
#include "../core/ssh/session.h"
#include "../core/ssh/key_utils.h"
#include "stdio.h"
#include "stdlib.h"

static void out_msg(char *msg);

#define test_port 2456

int test_server() {
  struct ssh_conn server;

  server.port = test_port;

  int rc;
  rc = ssh_pki_import_privkey_file(server_privkey_file, NULL, NULL, NULL, &server.key);

  if(rc != SSH_OK) {
    out_msg("bad import privkey");
    return -1; 
  }

  rc = init_server_session(&server); 
  
  if(rc != SSH_OK) {
    out_msg("error init server");
    return -1; 
  }

  fprintf(stdout, "%s/n", "init server success");

  ssh_conn_session_close(&server, "end test");
  ssh_key_free(server.key);

  return 0;
}

int test_client() {
  struct ssh_conn client;
 
  client.port = test_port;

  if(ssh_pki_import_pubkey_file(user_pubkey_file, &client.key) != SSH_OK) {
    out_msg("bad import pubkey");
    return -1; 
  }

  if(init_user_session(&client, "127.0.0.1") < 0) {
    return -1;
  }

  fprintf(stdout, "%s\n", "init client success");

  ssh_conn_session_close(&client, "end test");
  ssh_key_free(client.key);

  return 0;
}


static void out_msg(char *msg) {
  fprintf(stdout, "%s\n", msg);
}
