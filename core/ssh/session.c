#include "session.h"
#include "../logging.h"
#include "auth.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <string.h>
#include <stdio.h>

static int ssh_session_bind(struct ssh_conn *server, ssh_bind *bind);
static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind);

int init_user_session(struct ssh_conn *user, const char *host) {
  if(user == NULL) return -1;

  const char *error_message;
  int rc;
  ssh_key privkey = NULL;

  user->session = ssh_new();
  if(user->session == NULL)
    goto failure_connect;
 
  rc = ssh_options_set(user->session, SSH_OPTIONS_HOST, host);
  if(rc != SSH_OK) goto failure_connect;

  rc = ssh_options_set(user->session, SSH_OPTIONS_PORT, &user->port);
  if(rc != SSH_OK) goto failure_connect;

//TODO:: set knownhost for normal mode
#ifdef TEST
  rc = ssh_options_set(user->session, SSH_OPTIONS_KNOWNHOSTS, "test-res/known_hosts");
#endif
  if(rc != SSH_OK) goto failure_connect;

  if(ssh_connect(user->session) != SSH_OK)
    goto failure_connect; 

  if(verify_host(user->session) < 0)
   goto failure_connect;

  rc = ssh_userauth_try_publickey(user->session, NULL, user->key);
  if(rc != SSH_AUTH_SUCCESS)
    goto failure_connect;

#ifdef TEST
  rc = ssh_pki_import_privkey_file("test-res/client", NULL, NULL, NULL, &privkey);
#endif

  if(rc != SSH_OK) goto failure_connect;

  rc = ssh_userauth_publickey(user->session, NULL, privkey);
  if(rc != SSH_AUTH_SUCCESS)
    goto failure_connect;
  ssh_key_free(privkey);

  return 0;

failure_connect:
  if(privkey != NULL) ssh_key_free(privkey);
  error_message = ssh_get_error(user->session);
  log_error(user->session, error_message);
  ssh_conn_session_close(user);
  return -1;
}

int init_server_session(struct ssh_conn *server) {
  if(server == NULL) return -1;
  
  int rc;
  const char *error_message;
  ssh_bind bind = NULL;
  ssh_event auth = NULL;
  if(ssh_session_bind(server, &bind) < 0) {
    error_message = "Bind failure";
    goto failure_init;
  }

  if(ssh_session_accept(server, bind) < 0) { 
    error_message = "Breake connection";
    goto failure_init;
  }

  int auth_step = 0;
  int user_auth = SSH_AUTH_AGAIN;
  struct ssh_server_callbacks_struct cb;
  memset(&cb, 0, sizeof(cb));
  cb.userdata = &user_auth;
  cb.auth_pubkey_function = verify_user;
  ssh_callbacks_init(&cb);
  rc = ssh_set_server_callbacks(server->session, &cb);
  if(rc != SSH_OK) {
    error_message = ssh_get_error(server->session);
    goto failure_init;
  }

  rc = ssh_handle_key_exchange(server->session);
  if(rc != SSH_OK) {
    error_message = ssh_get_error(server->session);
    goto failure_init;
  }

  auth = ssh_event_new();
  if(auth == NULL) {
   error_message = "ssh_event_new() failed";
   goto failure_init;
  }

  rc = ssh_event_add_session(auth, server->session);
  if(rc != SSH_OK) {
    error_message = ssh_get_error(server->session);
    goto failure_init;
  }

//INFO:: wait ssh_userauth_try_publickey, ssh_userauth_publickey
  while (ssh_event_dopoll(auth, -1) == SSH_OK) {
    if (user_auth == SSH_AUTH_SUCCESS)
      break;
    else if (user_auth == SSH_AUTH_DENIED)
      goto failure_init;
}

  ssh_event_free(auth);

  return SSH_OK;

failure_init:
  ssh_event_free(auth);
  log_error(server->session, error_message);
  ssh_conn_session_close(server);
  return -1;
}

static int ssh_session_bind(struct ssh_conn *server, ssh_bind *bind) {
  if(server == NULL) return -1;

  const char *error_message;

  *bind = ssh_bind_new();
  if(bind == NULL) {
    goto failure_bind;
  }

  int rc;
  //TODO:: set addr from config env
  rc = ssh_bind_options_set(*bind, SSH_BIND_OPTIONS_BINDADDR, "127.0.0.1");

  if (rc < 0) goto failure_bind;

  rc = ssh_bind_options_set(*bind, SSH_BIND_OPTIONS_BINDPORT, &server->port);
  if (rc < 0) goto failure_bind;

  rc = ssh_bind_options_set(*bind, SSH_BIND_OPTIONS_IMPORT_KEY, server->key);
  if (rc < 0) goto failure_bind;

  if(ssh_bind_listen(*bind) < 0)
    goto failure_bind;

  return 0;

failure_bind:
  error_message = ssh_get_error(&bind);
  ssh_bind_free(*bind);
//TODO:: logging
  fprintf(stdout, "%s\n", error_message);
  return -1;
}

static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind) {
  if(server == NULL) return -1;

  int rc;
  const char *error_message;

  server->session = ssh_new();
  if(server->session == NULL) return -1;

  //TODO:: change logging
  fprintf(stdout, "%s\n", "start accept");
  
  rc = ssh_bind_accept(bind, server->session); 
  if (rc != SSH_OK) {
    fprintf(stdout, "%s\n", "Accept timeout error");
    error_message = "Accepet timeout error";
    return -1;
  }
  return 0;
}

void ssh_conn_session_close(struct ssh_conn *peer) {
  log_info(peer->session, "close session");

  if (peer->session) {
    ssh_disconnect(peer->session);
    ssh_free(peer->session);
    peer->session = NULL;
  }
}
