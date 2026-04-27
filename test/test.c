#include "test.h"
#include "../core/ssh/channel.h"
#include "../core/ssh/session.h"
#include "../core/ssh/key_utils.h"
#include "../core/OS/threads.h"
#include "../core/context_utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"

static void out_msg(const char *msg);
void static wait_recv(struct ssh_conn *peer);
void static wait_send(struct ssh_conn *peer, char *msg);

int test_server() {
  int rc;
  struct ssh_conn *server = NULL;
  thread_t tid;

  server = init_server_session(); 

  if(server == NULL) {
    out_msg("error init server");
    goto failure_cleanup; 
  }

  fprintf(stdout, "%s\n", "init server success");

  start(server);

  out_msg("trans messages\n");

  wait_recv(server);
  wait_send(server, "ne pravda\0");

  stop(server);
  ssh_conn_session_close(server);

  return 0;

failure_cleanup:
  if(server != NULL) ssh_conn_session_close(server);
  return -1;
}

int test_client() {
  int rc;
  struct ssh_conn *client = NULL;
  const char *host = "127.0.0.1";
  thread_t tid;

  client = init_user_session(host);
   
  if(client == NULL) {
    out_msg("error init client");
    goto failure_cleanup;
  }

  out_msg("init client success");

  start(client);

  out_msg("trans messages\n");

  wait_send(client, "server govno\0");
  wait_recv(client);

  stop(client);
  ssh_conn_session_close(client);
  out_msg("end client test");
  return 0;

failure_cleanup:
  if(client != NULL) ssh_conn_session_close(client);
  return -1;
}

static void out_msg(const char *msg) {
  fprintf(stdout, "%s\n", msg);
}

void static wait_recv(struct ssh_conn *peer) {
  out_msg("wait_recv");
  struct channel_pair *pair;
  char buf[CONTEXT_SIZE];
  size_t len;

  for(int i = 0; i < MAX_CHANNELS; i++) {
    pair = &peer->data.channels_data[i];
    len = read_data(pair, buf);
    fprintf(stdout, "%s%i%s%s\n", "out data from channel ", i, ": ", buf);
    clear_readed(pair); 
  }
}

void static wait_send(struct ssh_conn *peer, char *msg) {
  out_msg("wait_send");
  struct channel_pair *pair;
  
  for(int i = 0; i < MAX_CHANNELS; i++) {
    pair = &peer->data.channels_data[i];
    write_data(pair, msg, strlen(msg)+1);
    fprintf(stdout, "%s%i%s%s\n", "send data to channel ", i, ": ", msg);
  }
}
