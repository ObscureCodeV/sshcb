#include "session.h"
#include "../OS/socket_utils.h"
#include "auth.h"
#include "config.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <string.h>
#include <stdio.h>

static int ssh_session_bind(struct Server *conn);
static int ssh_session_accept(struct Server *conn);

int init_user_session(struct User *conn, const char *host) {
  if(conn == NULL) return -1;

  const char *error_message;

  conn->session = ssh_new();
  if(conn->session == NULL)
    goto failure_connect;
  
  ssh_options_set(conn->session, SSH_OPTIONS_HOST, host);
  ssh_options_set(conn->session, SSH_OPTIONS_PORT, &conn->port);

  if(ssh_connect(conn->session) != SSH_OK)
    goto failure_connect; 

  if(verify_host(conn->session) < 0)
   goto failure_connect;

  if(ssh_userauth_publickey(conn->session, NULL, conn->key) < 0)
    goto failure_connect;

  return 0;

failure_connect:
  error_message = ssh_get_error(conn->session); 
  user_session_close(conn, error_message);
  return -1;
}

int init_server_session(struct Server *conn) {
  if(conn == NULL) return -1;
  
  int rc;
  const char *error_message;
  if(ssh_session_bind(conn) < 0) {
    error_message = "Bind failure";
    goto failure_init;
  }

  int user_auth = SSH_AUTH_AGAIN;
  struct ssh_server_callbacks_struct cb;
  memset(&cb, 0, sizeof(cb));
  cb.userdata = &user_auth;
  cb.auth_pubkey_function = verify_user;
  ssh_callbacks_init(&cb);
  rc = ssh_set_server_callbacks(conn->session, &cb);
  if(rc != SSH_OK) {
    error_message = ssh_get_error(&cb);
    goto failure_init;
  }

  if(ssh_session_accept(conn) < 0) { 
    error_message = "Breake connection";
    goto failure_init;
  } 

  return 0;

failure_init:
  server_session_close(conn, error_message);
  return -1;
}

static int ssh_session_bind(struct Server *conn) {
  if(conn == NULL) return -1;

  const char *error_message;

  conn->bind = ssh_bind_new();
  if(conn->bind == NULL) {
    goto failure_bind;
  }

  ssh_bind_options_set(conn->bind, SSH_BIND_OPTIONS_BINDPORT, &conn->port);
  ssh_bind_options_set(conn->bind, SSH_BIND_OPTIONS_HOSTKEY, conn->key);

  if(ssh_bind_listen(conn->bind) < 0)
    goto failure_bind;

  return 0;

failure_bind:
  error_message = ssh_get_error(conn->bind);
  ssh_bind_free(conn->bind);
//TODO:: logging
  fprintf(stdout, "%s\n", error_message);
  return -1;
}

static int ssh_session_accept(struct Server *conn) {
  if(conn == NULL) return -1;

  const char *error_message;

  cross_socket bind_fd = ssh_bind_get_fd(conn->bind); 
  int rc;
  fd_set fd_in, fd_out;
  while(1) { 
    rc = select_timeout(bind_fd, fd_in, fd_out, ACCEPT_TIMEOUT_MS);
    if(rc == 1) break;
    if(rc == 0) continue;
    else {
      error_message = "Select timeout error";
      return -1;
    }
  }

  conn->session = ssh_new();
  if(conn->session == NULL) return -1;
  rc = ssh_bind_accept(conn->bind, conn->session); 
  if (rc == SSH_ERROR) {
    error_message = ssh_get_error(conn->bind);
    ssh_free(conn->session);
    conn->session = NULL;
    return -1;
  } 

  return 0;

failure_accept:
  //TODO:: change logging
  fprintf(stderr, "[ACCEPT] %s\n", ssh_get_error(conn->bind));
  if(conn->session != NULL) {
    ssh_free(conn->session);
    conn->session = NULL;
  }
  return -1;
}

void user_session_close(struct User *conn, const char *description) {

  if (conn->session) {
    ssh_disconnect(conn->session);
    ssh_free(conn->session);
    conn->session = NULL;
  }

  //TODO:: logging
  fprintf(stdout, "%s\n", description);
}

void server_session_close(struct Server *conn, const char *description) {

  if (conn->session) {
    ssh_disconnect(conn->session);
    ssh_free(conn->session);
    conn->session = NULL;
  }

  if(conn->bind) {
    ssh_bind_free(conn->bind);
  }

  //TODO:: logging
  fprintf(stdout, "%s\n", description);
}
