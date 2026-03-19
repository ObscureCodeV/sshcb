#include "session.h"
#include "../OS/socket_utils.h"
#include "auth.h"
#include "config.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <string.h>
#include <stdio.h>

static int ssh_session_bind(struct ssh_conn *server, ssh_bind bind);
static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind);

int init_user_session(struct ssh_conn *user, const char *host) {
  if(user == NULL) return -1;

  const char *error_message;

  user->session = ssh_new();
  if(user->session == NULL)
    goto failure_connect;
  
  ssh_options_set(user->session, SSH_OPTIONS_HOST, host);
  ssh_options_set(user->session, SSH_OPTIONS_PORT, &user->port);

  if(ssh_connect(user->session) != SSH_OK)
    goto failure_connect; 

  if(verify_host(user->session) < 0)
   goto failure_connect;

  if(ssh_userauth_publickey(user->session, NULL, user->key) < 0)
    goto failure_connect;

  return 0;

failure_connect:
  error_message = ssh_get_error(user->session); 
  ssh_conn_session_close(user, error_message);
  return -1;
}

int init_server_session(struct ssh_conn *server) {
  if(server == NULL) return -1;
  
  int rc;
  const char *error_message;
  ssh_bind bind = NULL;
  if(ssh_session_bind(server, bind) < 0) {
    error_message = "Bind failure";
    goto failure_init;
  }

  int user_auth = SSH_AUTH_AGAIN;
  struct ssh_server_callbacks_struct cb;
  memset(&cb, 0, sizeof(cb));
  cb.userdata = &user_auth;
  cb.auth_pubkey_function = verify_user;
  ssh_callbacks_init(&cb);
  rc = ssh_set_server_callbacks(server->session, &cb);
  if(rc != SSH_OK) {
    error_message = ssh_get_error(&cb);
    goto failure_init;
  }

  if(ssh_session_accept(server, bind) < 0) { 
    error_message = "Breake connection";
    goto failure_init;
  } 

  return 0;

failure_init:
  ssh_conn_session_close(server, error_message);
  return -1;
}

static int ssh_session_bind(struct ssh_conn *server, ssh_bind bind) {
  if(server == NULL) return -1;

  const char *error_message;

  bind = ssh_bind_new();
  if(bind == NULL) {
    goto failure_bind;
  }

  ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BINDPORT, &server->port);
  ssh_bind_options_set(bind, SSH_BIND_OPTIONS_HOSTKEY, server->key);

  if(ssh_bind_listen(bind) < 0)
    goto failure_bind;

  return 0;

failure_bind:
  error_message = ssh_get_error(bind);
  ssh_bind_free(bind);
//TODO:: logging
  fprintf(stdout, "%s\n", error_message);
  return -1;
}

static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind) {
  if(server == NULL) return -1;

  const char *error_message;

  cross_socket bind_fd = ssh_bind_get_fd(bind); 
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

  server->session = ssh_new();
  if(server->session == NULL) return -1;
  rc = ssh_bind_accept(bind, server->session); 
  if (rc == SSH_ERROR) {
    error_message = ssh_get_error(bind);
    ssh_free(server->session);
    server->session = NULL;
    return -1;
  } 

  return 0;

failure_accept:
  //TODO:: change logging
  fprintf(stderr, "[ACCEPT] %s\n", ssh_get_error(bind));
  if(server->session != NULL) {
    ssh_free(server->session);
    server->session = NULL;
  }
  return -1;
}

void ssh_conn_session_close(struct ssh_conn *peer, const char *description) {

  if (peer->session) {
    ssh_disconnect(peer->session);
    ssh_free(peer->session);
    peer->session = NULL;
  }

  //TODO:: logging
  fprintf(stdout, "%s\n", description);
}
