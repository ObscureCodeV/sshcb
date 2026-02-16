#include "connection.h"
#include <libssh2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int ssh_session_connect(ConnectedData *conn, const char *host, int port) {

  memset(conn, 0, sizeof(ConnectedData));
  
  conn->sock = socket(AF_INET, SOCK_STREAM, 0);
  if(conn->sock == LIBSSH2_INVALID_SOCKET) {
    // сообщение об ошибке
    goto failure_connect;
  }

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin))
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  if(inet_pton(AF_INET, host, &sin.sin_addr)!=1) {
    goto failure;
  }

  if(connect(conn->sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in))) {
    // сообщения об ошибке
    goto failure_connect;
  }

  conn->session = libssh2_session_init();
  if(!conn->session) {
    // сообщение об ошибке
    goto failure_connect;
  }

  if(libssh2_session_handshake(conn->session, conn->sock)) {
    // сообщения об ошибке
    goto failure_connect;
  }

  const char *fp = libssh2_hostkey_hash(conn->session, LIBSSH2_HOSTKEY_HASH_SHA256);
  if (fp) {
    strncpy(conn->fingerprint, fp, sizeof(conn->fingerprint) - 1);
    conn->fingerprint[sizeof(conn->fingerprint) - 1] = '\0';
  }

  return 0;

failure_connect:
    if(conn->session) {
        libssh2_session_disconnect(conn->session, "Connected failed");
        libssh2_session_free(conn->session);

    }
    if(conn->sock != LIBSSH2_INVALID_SOCKET) {
        shutdown(conn->sock, 2);
        LIBSSH2_SOCKET_CLOSE(conn->sock);
    }
    return 1;
}

int ssh_session_accept(ConnectedData *conn, int listen_sock);
