#include "test.h"
#include "../core/ssh/channel.h"
#include "../core/ssh/session.h"
#include "../core/ssh/key_utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void out_msg(const char *msg);
static void fill_channels(struct ssh_conn *peer, const char *msg);
static void out_channels_data(struct ssh_conn *peer);

int test_server() {
  int rc;
  struct ssh_conn *server = NULL;
  ssh_event recv;
  int recv_counter = 0;

  server = init_server_session(); 

  if(server == NULL) {
    out_msg("error init server");
    goto failure_cleanup; 
  }

  fprintf(stdout, "%s\n", "init server success");

  recv = ssh_event_new();
  if(recv == NULL) {
   out_msg("ssh_event_new() failed");
   goto failure_cleanup; 
  }

  rc = ssh_event_add_session(recv, server->session);
  if(rc != SSH_OK) {
   out_msg("ssh_event_add_session failed");
   goto failure_cleanup; 
  }

  while (1) {
    rc = ssh_event_dopoll(recv, -1);
    if(rc != SSH_OK) break;
    if(recv_counter == MAX_CHANNELS)
      break;
    if (data_filled(&server->data.context[recv_counter]))
      recv_counter++;
  }

  out_channels_data(server);

  ssh_conn_session_close(server);
  ssh_event_free(recv);
  return 0;

failure_cleanup:
  if(server != NULL) ssh_conn_session_close(server);
  if(recv != NULL) ssh_event_free(recv);
  return -1;
}

int test_client() {
  int rc;
  struct ssh_conn *client;
  const char *host = "127.0.0.1";

  client = init_user_session(host);
   
  if(client == NULL) {
    out_msg("error init client");
    goto failure_cleanup;
  }

  out_msg("init client success");

  rc = init_user_channels(client);
  if(rc != SSH_OK) {
    out_msg("error init user channels");
    goto failure_cleanup; 
  }

  fill_channels(client, "test message");

  ssh_conn_session_close(client);
  out_msg("end client test");
  return 0;

failure_cleanup:
  ssh_conn_session_close(client);
  return -1;
}

static void out_msg(const char *msg) {
  fprintf(stdout, "%s\n", msg);
}

static void fill_channels(struct ssh_conn *peer, const char *msg) {
  for(int i = 0; i < MAX_CHANNELS; i++) {
    memcpy(peer->data.context[i].data, msg, strlen(msg));
    peer->data.context[i].data_len = strlen(msg);
    send_data(&peer->data.channels[i], &peer->data.context[i]);
  }
}

static void out_channels_data(struct ssh_conn *peer) {
  for(int i = 0; i < MAX_CHANNELS; i++) {
    fprintf(stdout, "%s\n", peer->data.context[i].data);
  }
}
