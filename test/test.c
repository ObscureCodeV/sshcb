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

  thread_create(&tid, session_thread, server);
  thread_detach(tid);

  out_msg("trans messages\n");

  wait_recv(server);
  wait_send(server, "ne pravda");

  ssh_conn_session_close(server);
  return 0;

failure_cleanup:
  if(server != NULL) ssh_conn_session_close(server);
  return -1;
}

int test_client() {
  int rc;
  struct ssh_conn *client;
  const char *host = "127.0.0.1";
  thread_t tid;

  client = init_user_session(host);
   
  if(client == NULL) {
    out_msg("error init client");
    goto failure_cleanup;
  }

  out_msg("init client success");

  rc = init_user_channels(client);
  if(rc == SSH_ERROR)
    goto failure_cleanup;

  thread_create(&tid, session_thread, client);
  thread_detach(tid);

  out_msg("trans messages\n");

  wait_send(client, "server govno");
  wait_recv(client);

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

//TODO:: add cond
void static wait_recv(struct ssh_conn *peer) {
  char *buf;
  size_t len;

  struct channel_pair *pair;
  for(int i = 0; i < MAX_CHANNELS;) {

    mutex_lock(&peer->data.mutex);
    if(peer->data.active_channels != i) {
      mutex_unlock(&peer->data.mutex);
      continue;
    }
    mutex_unlock(&peer->data.mutex);

    pair = &peer->data.channels_data[i];
    mutex_lock(&pair->ctx.mutex);
    if(pair->ctx.state == STATE_DATA_READY) {
      mutex_unlock(&peer->data.mutex);
      read_data(pair, &buf, &len);
      fprintf(stdout, "%s%i%s%s\n", "out data from channel ", i, ": ", buf);
    }
    else
      mutex_unlock(&peer->data.mutex);
  }
}

//TODO:: add cond
void static wait_send(struct ssh_conn *peer, char *msg) {
  char buf[CONTEXT_SIZE];
  struct channel_pair *pair;
  for(int i = 0; i < MAX_CHANNELS;) {

    mutex_lock(&peer->data.mutex);
    if(peer->data.active_channels != i) {
      mutex_unlock(&peer->data.mutex);
      continue;
    }
    mutex_unlock(&peer->data.mutex);

    pair = &peer->data.channels_data[i];
    mutex_lock(&pair->ctx.mutex);
    if(pair->ctx.state == STATE_DATA_READY || pair->ctx.state == STATE_RECV_LEN) {
      mutex_unlock(&peer->data.mutex);
      write_data(pair, msg, strlen(msg));
      fprintf(stdout, "%s%i%s%s\n", "send data from channel ", i, ": ", buf);
    }
    else
      mutex_unlock(&peer->data.mutex);
  }
}
