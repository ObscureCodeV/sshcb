#include "daemon_utils.h"
#include "daemon_wrapper.h"
#include "../common/socket.h"
#include "../core/ssh/session.h"
#include "../core/context_utils.h"
#include "../core/ssh/data.h"
#include <libssh/libssh.h>
#include <stdio.h>


static void init_request(struct ssh_conn *conn, ipc_msg_t *packet);
static void close_request(struct ssh_conn *conn, ipc_msg_t *packet);
static void send_request(struct ssh_conn *conn, ipc_msg_t *packet);
static void read_request(struct ssh_conn *conn, ipc_msg_t *packet);

int daemon_main(void) {
#ifdef TEST
  ssh_set_log_level(SSH_LOG_PACKET);
#endif
  ssh_init();
  struct ssh_conn *conn = allocate_buffer();
  init_contexts(conn);
  
  socket_t server_sock = create_server_socket(SOCKET_PATH);
  if (server_sock == INVALID_SOCKET_VAL) {
    fprintf(stderr, "Failed to create socket\n");
    ssh_finalize();
    return 1;
  }

  printf("SSHCB daemon started on %s\n", SOCKET_PATH);

  while (daemon_is_running()) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server_sock, &readfds);
                                  
    struct timeval tv = {1, 0};  // timeout 1 sec
    int rc = select(server_sock + 1, &readfds, NULL, NULL, &tv);
    if (rc > 0 && FD_ISSET(server_sock, &readfds)) {
      socket_t client_sock = accept(server_sock, NULL, NULL);
      if (client_sock != INVALID_SOCKET_VAL) {
        ipc_msg_t msg;
        if (recv_message(client_sock, &msg) == 0) {
          handle_request(conn, &msg);
#ifdef TEST
          printf("%s\n", msg.data);
          printf("%d\n", msg.is_success);
#endif
          send_message(client_sock, &msg);
        }
        close_socket(client_sock);
      }
    }
  }
  close_socket(server_sock);
  unlink(SOCKET_PATH);
  stop(conn);
  ssh_conn_session_close(conn);
  
  printf("SSHCB daemon stopped\n");
  ssh_finalize();
  return 0;
}

void handle_request(struct ssh_conn *conn, ipc_msg_t *packet) {
  packet->is_success = 0;
  cmd_type_t command = packet->type;
  void (*request_func) (struct ssh_conn *, ipc_msg_t * ) = NULL;

  switch(command) {
    case CMD_INIT_CLIENT:
    case CMD_INIT_SERVER:
     request_func = init_request;
     break;

    case CMD_SESSION_CLOSE:
     request_func = close_request;
     break;

    case CMD_SEND:
     request_func = send_request;
     break;

    case CMD_READ:
     request_func = read_request;
     break;

    default:
     strcpy(packet->data, "UNKNOWN COMMAND\n");
     packet->data_len = strlen(packet->data);
     return;
  }

  request_func(conn, packet);
}

static void init_request(struct ssh_conn *conn, ipc_msg_t *packet) {
  conn_state_t state = conn->data.thread_state;
  char *message = packet->data;
  cmd_type_t command = packet->type;

  if(state == IS_RUNNED) {
    strcpy(message, "Yet started!");
    goto failure;
  }

  if(state == NOT_STARTED || state == IS_STOPPED) {
    conn_state_t (*init_func)(struct ssh_conn *, const char*); 
    if(command == CMD_INIT_CLIENT) init_func = init_user_session;
    else init_func = init_server_session;
    state = init_func(conn, packet->data);

    if(state != IS_IDLE) {
      strcpy(message, "Error init!");
      goto failure;
    }

    state = start(conn);
    if(state != IS_RUNNED) {
      strcpy(message, "Start error!");
      goto failure;
    }
    strcpy(message, "Success init!");
  }

  else {
    strcpy(message, "Session not consistend!");
    goto failure;
  }

  packet->is_success = 1;

failure:
   packet->data_len = strlen(packet->data);
}

static void close_request(struct ssh_conn *conn, ipc_msg_t *packet) {
  conn_state_t state = conn->data.thread_state;
  char *message = packet->data;
  
  if(state != IS_RUNNED) {
    strcpy(message, "Session not runned!");
    goto failure;
  }

  state = stop(conn);
  if(state != IS_STOPPED) {
    strcpy(message, "Session not consistend!");
    goto failure;
  }

  state = ssh_conn_session_close(conn);
  if(state != IS_CLOSE) {
    strcpy(message, "Session not consistend!");
    goto failure;
  }

   packet->is_success = 1;
failure:
   packet->data_len = strlen(packet->data);
}


static void send_request(struct ssh_conn *conn, ipc_msg_t *packet) {
  char *message = packet->data;
  int rc;

  clear(conn, packet->channel);
  rc = write_data(conn, packet->channel, packet->data, packet->data_len);
  
  if(rc == -1) {
    strcpy(message, "WRITE DATA ERROR! USE CORRECT CHANNEL!\n");
  }
  else if(rc == 0) {
    strcpy(message, "DATA CAN'T WRITE!\n");
  }
  else {
    packet->is_success = 1;
    strcpy(message, "DATA WRITTEN!\n");
  }

  packet->data_len = strlen(message);
}

static void read_request(struct ssh_conn *conn, ipc_msg_t *packet) {
  int rc;

  rc = read_data(conn, packet->channel, packet->data); 
  if(rc == -1) {
    strcpy(packet->data, "READ DATA ERROR! USE CORRECT CHANEL!\n");
  }
  else if(rc == 0) {
    strcpy(packet->data, "DATA CAN'T RECV\n");
  }
  else {
    packet->is_success = 1;
  }
  packet->data_len = strlen(packet->data);
}
