#include "test.h"
#include "../core/ssh/session.h"
#include "../core/ssh/key_utils.h"
#include "stdio.h"
#include "stdlib.h"

static void out_msg(char *msg);

int test_server() {
  struct ssh_conn *server = init_server_session(); 
  
  if(server == NULL) {
    out_msg("error init server");
    return -1; 
  }

  fprintf(stdout, "%s\n", "init server success");
  ssh_conn_session_close(server);

  return 0;
}

int test_client() {
  struct ssh_conn *client = init_user_session("127.0.0.1");
    
  if(client == NULL) {
    out_msg("error init client");
    return -1; 
  }

  fprintf(stdout, "%s\n", "init client success");

  ssh_conn_session_close(client);

  return 0;
}


static void out_msg(char *msg) {
  fprintf(stdout, "%s\n", msg);
}
