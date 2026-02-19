#include "data.h"
#include "channel.h"
#include <libssh2.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void push_back(struct ConnectedData *conn, LIBSSH2_CHANNEL *new_channel);

int init_first_channel(struct ConnectedData *conn) {
  conn->num_channels = 0;
  conn->channels = NULL;
  if(!add_channel(conn)) return 1;
  return 0;
}


int add_channel(struct ConnectedData *conn) {
  char *error_message;
  int errmsg_len;

  LIBSSH2_CHANNEL *new_channel = libssh2_channel_open_session(conn->session);
  if(new_channel == NULL) goto failure_open_channel;
  push_back(conn, new_channel);

  return 0;

failure_open_channel:
  libssh2_session_last_error(conn->session, &error_message, &errmsg_len, 0);
  //TODO:: change logging
  fprintf(stdout, "%s", error_message);
  return 1;
}

int remove_channel(struct ConnectedData *conn) {
  char *error_message;
  int errmsg_len;

  int idx = conn->num_channels - 1;
  if(!libssh2_channel_close(conn->channels[idx])) goto failure_close_channel;
  libssh2_channel_free(conn->channels[idx]);
  if(conn->channels[idx] != NULL) goto failure_close_channel;
  conn->num_channels--;

  return 0;
  
failure_close_channel:  
  libssh2_session_last_error(conn->session, &error_message, &errmsg_len, 0);
  //TODO:: change logging
  fprintf(stdout, "%s", error_message);
  return 1;
}

static void push_back(struct ConnectedData *conn, LIBSSH2_CHANNEL *new_channel) {
    int new_size = conn->num_channels + 1;
    
    LIBSSH2_CHANNEL **tmp = realloc(conn->channels, new_size * sizeof(LIBSSH2_CHANNEL *));
    
    if (tmp == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return;
    }

    conn->channels = tmp;
    conn->channels[conn->num_channels] = new_channel;
    conn->num_channels = new_size;
}
