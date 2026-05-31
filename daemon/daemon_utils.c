#include "daemon_utils.h"
#include "daemon_wrapper.h"
#include "../common/socket.h"
#include "../core/ssh/session.h"
#include "../core/context_utils.h"
#include <libssh/libssh.h>
#include <stdio.h>

void handle_request(struct ssh_conn **conn, ipc_msg_t *packet) {
  int rc;
  packet->is_success = 0;

  switch(packet->type) {
    case CMD_SEND:
      rc = write_data(*conn, packet->channel, packet->data, packet->data_len); 
      if(rc == -1) {
//INFO:: packet->data have size equal CONTEXT_SIZE
        strcpy(packet->data, "NEED INIT SESSION AND CHOOSE CORRECT CHANNEL\0");
      }
      else if(rc == 0) {
        strcpy(packet->data, "DATA CAN'T SEND\0");
      }
      else {
        packet->is_success = 1;
      }
      packet->data_len = strlen(packet->data);
      break;

    case CMD_READ:
      rc = read_data(*conn, packet->channel, packet->data); 
      if(rc == -1) {
        strcpy(packet->data, "NEED INIT SESSION AND CHOOSE CORRECT CHANNEL\0");
      }
      else if(rc == 0) {
        strcpy(packet->data, "DATA CAN'T RECV\0");
      }
      else {
        packet->is_success = 1;
      }
      packet->data_len = strlen(packet->data);
      break;

    case CMD_CLEAR:
      clear_readed(*conn, packet->channel);
      break;

    case CMD_INIT_CLIENT:
//INFO:: in this case packet->data used for ip
      *conn = init_user_session(packet->data);
      if(*conn == NULL) {
        strcpy(packet->data, "SESSION NOT INIT\0");
        packet->data_len = strlen(packet->data);
        return;
      }
      start(*conn);
      packet->is_success = 1;

      strcpy(packet->data, "SESSION INIT AND START\0");
      packet->data_len = strlen(packet->data);

      break;
    case CMD_INIT_SERVER:
//INFO:: in this case packet->data used for ip
      *conn = init_server_session(packet->data);
      if(*conn == NULL) {
        strcpy(packet->data, "SESSION NOT INIT\0");
        packet->data_len = strlen(packet->data);
        return;
      }
      start(*conn);
      packet->is_success = 1;

      strcpy(packet->data, "SESSION INIT AND START\0");
      packet->data_len = strlen(packet->data);
      
      break;

    case CMD_SESSION_CLOSE:
      stop(*conn);
      ssh_conn_session_close(*conn);
      packet->is_success = 1;
      break;

    default:
      strcpy(packet->data, "UNKNOWN COMMAND\0");
      packet->data_len = strlen(packet->data);
      break;
  }
  packet->is_daemon_response = 1;
}

int daemon_main(void) {
#ifdef TEST
  ssh_set_log_level(SSH_LOG_PACKET);
#endif
  ssh_init();
  struct ssh_conn *conn = NULL;
  
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
          handle_request(&conn, &msg);
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

