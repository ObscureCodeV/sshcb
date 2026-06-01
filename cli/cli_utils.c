#include "cli_utils.h"
#include "../common/socket.h"
#include "../common/protocol.h"
#include "../daemon/daemon_utils.h"
#include "../daemon/daemon_wrapper.h"

#include <stdio.h>
#include <stdlib.h>

void print_help(const char *prog) {
  printf("Usage: %s [options]\n", prog);

  printf("--daemon       Start daemon\n");

  printf("--init_client <server_ip>    Init client\n");
  printf("--init_server <listen_ip>    Init server\n");
  printf("--close         close session\n");

  printf("  -s, --send TEXT    Send text to clipboard\n");
  printf("  -r, --read         Read text from clipboard\n");

  printf("  -n, --channel N    Use channel N (0-9, default 0)\n");

  printf("  -h, --help         Show this help\n");
}

static int copy_data(ipc_msg_t *msg, const char *arg) {
  size_t len = strlen(arg);
  if (len >= CONTEXT_SIZE) {
    fprintf(stderr, "Error: argument too long (max %d)\n", CONTEXT_SIZE - 1);
    return -1;
  }
  strncpy(msg->data, arg, len);
  msg->data[len] = '\0';
  msg->data_len = len + 1;
  return 0;
}

void parse_command(int argc, char *argv[], ipc_msg_t *msg) {

  msg->type = CMD_NONE;
  msg->channel = 0;
  msg->data[0] = '\0';
  msg->data_len = 0;

  for(int i = 1; i < argc; i++) {
    const char *arg = argv[i];

    if(strcmp(arg, "--") == 0) break;

    if(strncmp(arg, "--", 2) == 0) {
      arg += 2;

      if(strcmp(arg, "daemon") == 0) msg->type = CMD_DAEMON;
      else if(strcmp(arg, "read") == 0) msg->type = CMD_READ;
      else if(strcmp(arg, "help") == 0) msg->type = CMD_HELP;
      else if(strcmp(arg, "close") == 0) msg->type = CMD_SESSION_CLOSE;

      else if(strcmp(arg, "channel") == 0) {
        arg += 8;
        if (++i >= argc) {
          fprintf(stderr, "Error: --channel requires value\n");
          return;
        }
        char *end;
        long v = strtol(argv[i], &end, 10);
        if (*end != '\0' || v < 0 || v > 9) {
          fprintf(stderr, "Error: channel must be 0-9\n");
          return;
        }
        msg->channel = (int)v;
      }
      else if(strcmp(arg, "init_client") == 0) {
        if (++i >= argc) {
          fprintf(stderr, "Error: --init_client requires connect ip\n");
          return;
        }
        msg->type = CMD_INIT_CLIENT;
        if(copy_data(msg, argv[i]) != 0)
          msg->type = CMD_NONE;
      }
      else if(strcmp(arg, "init_server") == 0) {
        if (++i >= argc) {
          fprintf(stderr, "Error: --init_server requires listen ip\n");
          return;
        }
        msg->type = CMD_INIT_SERVER;
        if(copy_data(msg, argv[i]) != 0)
          msg->type = CMD_NONE;
      }
      else if(strcmp(arg, "send") == 0) {
        if (++i >= argc) {
          fprintf(stderr, "Error: --send requires text\n");
          return;
        }
        msg->type = CMD_SEND;
        if(copy_data(msg, argv[i]) != 0)
          msg->type = CMD_NONE;
      }
    }

    else if (arg[0] == '-') {
      arg++;

      if(strcmp(arg, "r") == 0) msg->type = CMD_READ;
      else if(strcmp(arg, "h") == 0) msg->type = CMD_HELP;
    }

    else if(strcmp(arg, "s") == 0) {
      arg += 1;
      if (++i >= argc) {
        fprintf(stderr, "Error: --send requires text\n");
        return;
      }
      msg->type = CMD_SEND;
      copy_data(msg, argv[i]);
    } 
  }

  if(msg->type == CMD_NONE)
    msg->type = CMD_HELP;
}

int send_command(ipc_msg_t msg) {
  if(msg.type == CMD_HELP) {
    print_help("sshcb");
    return 0;
  }

  if(msg.type == CMD_DAEMON) {
    fprintf(stdout, "\nDaemon is running!");
    return daemon_run(daemon_main);
  }

  socket_t sock = create_client_socket(SOCKET_PATH);
  if (sock == INVALID_SOCKET_VAL) {
      fprintf(stderr, "\nDaemon not running.");
      return -1;
  }

  if(send_message(sock, &msg) != 0) {
    fprintf(stderr, "\nFailed to send request");
    close_socket(sock);
  }

  if(recv_message(sock, &msg) != 0) {
    fprintf(stderr, "\nFailed to recevie response");
    close_socket(sock);
    return -1;
  }

  close_socket(sock);

  if(!msg.is_daemon_response) {
    fprintf(stderr, "\nInvalid response");
    return -1;
  }

//INFO:: success = 0 for errors or 1 for success operations
  int return_value = msg.is_success ? 0 : -1;

//INFO:: out error message or read data
  if (fwrite(msg.data, 1, msg.data_len, stdout) != msg.data_len) {
    fprintf(stderr, "Failed to write to stdout\n");
    return -1;
  }
  fflush(stdout);

  return return_value;
}

