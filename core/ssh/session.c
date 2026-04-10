#include "session.h"
#include "channel.h"
#include "../config.h"
#include "../logging.h"
#include "auth.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <libssh/callbacks.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int ssh_session_bind(struct ssh_conn *server, ssh_bind *bind, const struct sshcb_config *cfg);
static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind);

struct ssh_conn* init_user_session(const char *host) {
  struct ssh_conn *user = malloc(sizeof(struct ssh_conn));

  const char *error_message = NULL;
  int rc;
  ssh_key privkey = NULL;
  const struct sshcb_config *cfg = NULL;

  user->session = ssh_new();
  if(user->session == NULL)
    goto failure_connect;

  cfg = sshcb_get_config();

  if(cfg == NULL) {
    error_message = "Failed get sshcb_config";
    goto failure_connect;
  }

  user->port = cfg->client_port;
  rc = ssh_pki_import_pubkey_file(cfg->client_pubkey_path, &user->key);
  if(rc != SSH_OK) {
    error_message = "Failed open client pubkey";
    goto failure_connect;
  }
 
  rc = ssh_options_set(user->session, SSH_OPTIONS_HOST, host);
  if(rc != SSH_OK) goto failure_connect;

  rc = ssh_options_set(user->session, SSH_OPTIONS_PORT, &user->port);
  if(rc != SSH_OK) goto failure_connect;

  rc = ssh_options_set(user->session, SSH_OPTIONS_KNOWNHOSTS, cfg->known_host_path);
  if(rc != SSH_OK) goto failure_connect;

  if(ssh_connect(user->session) != SSH_OK) {
//    error_message = "Failed connect to host";
    goto failure_connect; 
  }

  if(verify_host(user->session) < 0)
   goto failure_connect;

  rc = ssh_userauth_try_publickey(user->session, NULL, user->key);
  if(rc != SSH_AUTH_SUCCESS)
    goto failure_connect;

  rc = ssh_pki_import_privkey_file(cfg->client_privkey_path, NULL, NULL, NULL, &privkey);

  if(rc != SSH_OK) goto failure_connect;

  rc = ssh_userauth_publickey(user->session, NULL, privkey);
  if(rc != SSH_AUTH_SUCCESS)
    goto failure_connect;

  ssh_key_free(privkey);

  ssh_set_blocking(user->session, 0);

  return user;

failure_connect:
  if(privkey != NULL) ssh_key_free(privkey);
  if(error_message == NULL) error_message = ssh_get_error(user->session);
  log_error(user->session, error_message);
  ssh_conn_session_close(user);
  return NULL;
}

struct ssh_conn* init_server_session() {
  struct ssh_conn *server = malloc(sizeof(struct ssh_conn));
  
  int rc;
  const char *error_message;
  ssh_bind bind = NULL;
  ssh_event auth = NULL;
  const struct sshcb_config *cfg = NULL;

  cfg = sshcb_get_config();

  if(cfg == NULL) {
    error_message = "Failed get sshcb_config";
    goto failure_init;
  }

  server->port = cfg->server_port;
  rc = ssh_pki_import_privkey_file(cfg->server_privkey_path, NULL, NULL, NULL, &server->key);
  if(rc != SSH_OK) {
    error_message = "Failed open server privkey\n";
    goto failure_init;
  }

  if(ssh_session_bind(server, &bind, cfg) < 0) {
    error_message = "Bind failure";
    goto failure_init;
  }

  if(ssh_session_accept(server, bind) < 0) { 
    error_message = "Breake connection";
    goto failure_init;
  }

  server->data.active_channels = 0;

  struct ssh_server_callbacks_struct cb;
  memset(&cb, 0, sizeof(cb));
  cb.userdata = &server->data;
  cb.auth_pubkey_function = verify_user;
  cb.channel_open_request_session_function = server_channel_open;
//  cb.channel_close_request_session_function = server_channel_close;
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

  ssh_set_blocking(server->session, 0);

  return server;

failure_init:
  ssh_event_free(auth);
  log_error(server->session, error_message);
  ssh_conn_session_close(server);
  return NULL;
}

static int ssh_session_bind(struct ssh_conn *server, ssh_bind *bind, const struct sshcb_config *cfg) {
  if(server == NULL) return -1;
  if(cfg == NULL) return -1;

  const char *error_message;

  *bind = ssh_bind_new();
  if(bind == NULL) {
    goto failure_bind;
  }

  int rc;
  rc = ssh_bind_options_set(*bind, SSH_BIND_OPTIONS_BINDADDR, cfg->bind_address);
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
  log_error(server->session, error_message);
  return -1;
}

static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind) {
  if(server == NULL) return -1;

  int rc;

  server->session = ssh_new();
  if(server->session == NULL) return -1;

  log_info(server->session, "start accept");
  
  rc = ssh_bind_accept(bind, server->session); 
  if (rc != SSH_OK) {
    //TODO:: set timeout
    log_error(server->session, "Accepet timeout error");
    return -1;
  }
  return 0;
}

void ssh_conn_session_close(struct ssh_conn *peer) {
  log_info(peer->session, "close session");

  if(peer->session) {
    ssh_disconnect(peer->session);
    ssh_free(peer->session);
    peer->session = NULL;
  }
  if(peer->key) {
    ssh_key_free(peer->key);
  }
 
  mutex_t *m = NULL;
  for(int idx = 0; idx < MAX_CHANNELS; idx++) {
    *m = peer->data.context[idx].mutex;
    if(m)
      mutex_destroy(m);
  }
}
