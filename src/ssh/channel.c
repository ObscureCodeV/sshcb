#include "data.h"
#include "channel.h"
#include <libssh2.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int init_channel(struct ConnectedData *conn, int idx) {
  char *error_message;
  int errmsg_len;

  if(conn == NULL) return -1;

  if(idx < 0 || idx >= MaxChannelsNum) {
    error_message = "bad channel idx by open_channel";
    goto failure_open_channel;
  }

  conn->channels[idx] = libssh2_channel_open_session(conn->session);
  if(conn->channels[idx] == NULL) {
    libssh2_session_last_error(conn->session, &error_message, &errmsg_len, 0);
    goto failure_open_channel;
  }
  return 0;

failure_open_channel:
//TODO:: change logging
  fprintf(stdout, "%s", error_message);
  return -1;
}

int close_channel(struct ConnectedData *conn, int idx) {
  char *error_message;
  int errmsg_len;

  if(conn == NULL) return -1;
  
  if(idx < 0 || idx >= MaxChannelsNum) {
    error_message = "bad channel idx by close_channel";    
    //TODO:: change logging
    fprintf(stdout, "%s", error_message);
    return -1;
  }

  if(conn->channels[idx] == NULL) {
    return 0;
  }

  if(libssh2_channel_close(conn->channels[idx]) < 0) {
    goto failure_close_channel;
  }
  if(libssh2_channel_free(conn->channels[idx]) < 0) {
    goto failure_close_channel;
  }

  memset(&conn->context[idx], 0, sizeof(struct ChannelContext));
  conn->channels[idx] = NULL; 

  return 0;
  
failure_close_channel:  
  libssh2_session_last_error(conn->session, &error_message, &errmsg_len, 0);
  //TODO:: change logging
  fprintf(stdout, "%s", error_message);
  return -1;
}

int close_all_channel(struct ConnectedData *conn) {
  int ret = 0;
  for(int i = 0; i < MaxChannelsNum; i++) {
    if(conn->channels[i] != NULL) {
      if(close_channel(conn, i) != 0) {
        ret = -1;
      }
    }
  }
  return ret;
}
