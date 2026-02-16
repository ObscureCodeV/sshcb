#include "connection.h"
#include <libssh2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

static int ssh_session_handshake(ConnectedData *conn);

int ssh_session_connect(ConnectedData *conn, const char *host, int port) {

  memset(conn, 0, sizeof(ConnectedData));
  
  conn->sock = socket(AF_INET, SOCK_STREAM, 0);
  if(conn->sock == LIBSSH2_INVALID_SOCKET) {
    // сообщение об ошибке
    goto failure_connect;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if(inet_pton(AF_INET, host, &server_addr.sin_addr)!=1) {
    goto failure_connect;
  }
  
  if(ssh_session_handshake(conn)) goto failure_connect;

  return 0;

failure_connect:
    if(conn->session) {
        libssh2_session_disconnect(conn->session, "Connected failed");
        libssh2_session_free(conn->session);

    }
    if(conn->sock != LIBSSH2_INVALID_SOCKET) {
        shutdown(conn->sock, SHUT_RDWR);
        LIBSSH2_SOCKET_CLOSE(conn->sock);
    }
    return 1;
}

int ssh_session_accept(ConnectedData *conn, int listen_sock) {
  memset(conn, 0, sizeof(ConnectedData));
  struct sockaddr_in client_addr;
  socklen_t client_addr_size = sizeof(client_addr);
  bzero(&client_addr, sizeof(client_addr_size));

  conn->sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size);
  if (conn->sock == LIBSSH2_INVALID_SOCKET) {
    // error message
    goto failure_accept;
  }

  if(ssh_session_handshake(conn)) goto failure_accept;

  return 0;

failure_accept:
    if(conn->sock != LIBSSH2_INVALID_SOCKET) {
        shutdown(conn->sock, SHUT_RDWR);
        LIBSSH2_SOCKET_CLOSE(conn->sock);
    }
    return 1;
}

static int ssh_session_handshake(ConnectedData *conn) {    
  conn->session = libssh2_session_init();
  if (!conn->session) {
    // message error
    goto failure_handshake;
  }
    
  if (libssh2_session_handshake(conn->session, conn->sock)) {
    //message error
    goto failure_handshake;
  }
    
  const char *fp = libssh2_hostkey_hash(conn->session, LIBSSH2_HOSTKEY_HASH_SHA256);
  if (fp) {
    strncpy(conn->fingerprint, fp, sizeof(conn->fingerprint) - 1);
    conn->fingerprint[sizeof(conn->fingerprint) - 1] = '\0';
  }
  
  return 0;
    
failure_handshake:
    if (conn->session) {
        libssh2_session_disconnect(conn->session, "Connection failed");
        libssh2_session_free(conn->session);
        conn->session = NULL;
    }
    
    return 1;
}
