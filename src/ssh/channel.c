#include "data.h"
#include "channel.h"
#include <libssh/libssh.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

static void channel_timeout(ssh_channel channel);

int read_channel(ssh_channel channel, struct ChannelContext *context) {
  int bytes_read;
  int total_read = 0;
  int max_size = sizeof(context->data) - 1;

  while(1) {
    bytes_read = ssh_channel_read_nonblocking(channel, context->data + total_read, max_size - total_read, 0);
    if (bytes_read > 0) {
      total_read += bytes_read;
      if(total_read >= max_size) break;
    }
    else if(bytes_read == SSH_AGAIN) {
      channel_timeout(channel);
      continue;
    }
    else { //ERROR or EOF
      break;
    }
  }
  context->data[total_read] = '\0';
  context->data_len = total_read;
  return total_read;
}

int write_channel(ssh_channel channel, struct ChannelContext *context) {
  int bytes_written;
  int total_written = 0;
  int buffer_size = context->data_len;

 while(1) {
    bytes_written = ssh_channel_write(channel, context->data + total_written, buffer_size - total_written);
    if (bytes_written > 0) {
      total_written += bytes_written;
      if(total_written >= buffer_size) break;
    }
    else if(bytes_written == SSH_AGAIN) {
      channel_timeout(channel);
      continue;
    }
    else {
      break;
    }
  }
  ssh_channel_send_eof(channel);
  return total_written;
}

int init_channels(ssh_session session, ssh_channel *channels, int NumChannels) {
  for(int i = 0; i < NumChannels; i++) {
    channels[i] = ssh_channel_new(session);
    if(channels[i] == NULL) {
      return -1;
    }
  }
  return 0;
}

static void channel_timeout(ssh_channel channel) {
  ssh_session session = ssh_channel_get_session(channel);
  ssh_event event = ssh_event_new();
  ssh_event_add_session(event, session);
  ssh_event_dopoll(event, 100);
  ssh_event_free(event);
}


int shutdown_channels(ssh_session *session, ssh_channel *channels, int NumChannels) {
  for(int i = 0; i < NumChannels; i++) {
    ssh_channel_free(channels[i]);
    if(channels[i] == NULL) {
      return -1;
    }
  }
  return 0;
}

int open_channel(ssh_channel channel) {
  const char *error_message;

  if(ssh_channel_is_open(channel)) return 0;

  int rc;
  rc = ssh_channel_open_session(channel);

  if(rc == SSH_OK) return 0;

  error_message = ssh_get_error(channel); 
//TODO:: change logging
  fprintf(stdout, "%s", error_message);
  ssh_channel_close(channel);
  return -1;
}

int close_channel(ssh_channel channel) {
  const char *error_message;

  if(ssh_channel_is_closed(channel)) return 0;

  int rc;
  rc = ssh_channel_close(channel);

  if(rc == SSH_OK) return 0;

  error_message = ssh_get_error(channel); 
//TODO:: change logging
  fprintf(stdout, "%s", error_message);
  return -1;
}
