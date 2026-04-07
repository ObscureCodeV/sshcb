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
  struct ssh_conn *server = init_server_session(); 
  
  if(server == NULL) {
    out_msg("error init server");
    return -1; 
  }

  fprintf(stdout, "%s\n", "init server success");

  int recv_counter = 0;

  ssh_event recv = ssh_event_new();
  if(recv == NULL) {
   fprintf(stdout, "ssh_event_new() failed");
   return -1;
  }

  rc = ssh_event_add_session(recv, server->session);
  if(rc != SSH_OK) {
   fprintf(stdout, "ssh_event_add_session failed");
   return -1;
  }

  while (ssh_event_dopoll(recv, -1) == SSH_OK) {
    if (check_recv_success(&server->context[recv_counter]))
      recv_counter++;
    if(recv_counter == MaxChannelsNum)
      break;
  }

  out_channels_data(server);

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

  fill_channels(client, "test message");

  ssh_conn_session_close(client);

  return 0;
}


static void out_msg(const char *msg) {
  fprintf(stdout, "%s\n", msg);
}

static void fill_channels(struct ssh_conn *peer, const char *msg) {
  for(int i = 0; i < MaxChannelsNum; i++) {
    memcpy(peer->context[i].data, msg, strlen(msg));
    peer->context[i].data_len = strlen(msg);
    send_data(&peer->channels[i], &peer->context[i]);
  }
}

static void out_channels_data(struct ssh_conn *peer) {
  for(int i = 0; i < MaxChannelsNum; i++) {
    fprintf(stdout, "%s\n", peer->context[i].data);
  }
}
