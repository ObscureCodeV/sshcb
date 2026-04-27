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

static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind);
static void init_session_data(struct ssh_conn *peer);
static ssh_bind bind_init(struct ssh_conn *server, const struct sshcb_config *cfg, ssh_key *privkey);

void *session_thread(void *arg) {
  struct ssh_conn *peer = arg;
  ssh_event event = ssh_event_new();
  int rc;
  int should_stop = 0;

  if(event == NULL) {
    log_error(peer->session, "ssh_event_new() failerd");
    return NULL;
  }

  rc = ssh_event_add_session(event, peer->session);
  if(rc != SSH_OK) {
    log_error(peer->session, "event add failed");
    ssh_event_free(event);
    return NULL;
  }

  while(ssh_event_dopoll(event, 100) == SSH_OK) {
    mutex_lock(&peer->data.mutex);
    should_stop = (peer->data.thread_state == IS_STOPPING);
    mutex_unlock(&peer->data.mutex);

    if(should_stop) break;
  }

  ssh_event_free(event);

  mutex_lock(&peer->data.mutex);
  peer->data.thread_state = IS_STOPPED;
  cond_signal(&peer->data.cond);
  mutex_unlock(&peer->data.mutex);

  return NULL;
}

void start(struct ssh_conn *peer) {
  mutex_lock(&peer->data.mutex);
  if(peer->data.thread_state == IS_RUNNING) {
    mutex_unlock(&peer->data.mutex);
    return;
  }

  thread_create(&peer->data.tid, session_thread, peer);
  thread_detach(peer->data.tid);
  peer->data.thread_state = IS_RUNNING;

  mutex_unlock(&peer->data.mutex);
}

void stop(struct ssh_conn *peer) {
  mutex_lock(&peer->data.mutex);
            
  if (peer->data.thread_state != IS_RUNNING) {
    mutex_unlock(&peer->data.mutex);
    return;
  }
                
  peer->data.thread_state = IS_STOPPING;
  cond_signal(&peer->data.cond);
  mutex_unlock(&peer->data.mutex);

  mutex_lock(&peer->data.mutex); 
  while (peer->data.thread_state == IS_STOPPING) {
    cond_timedwait(&peer->data.cond, &peer->data.mutex, 5000);
  }
  peer->data.thread_state = IS_STOPPED;
  cond_signal(&peer->data.cond);
  mutex_unlock(&peer->data.mutex); 
}

struct ssh_conn* init_user_session(const char *host) {
  struct ssh_conn *user = malloc(sizeof(struct ssh_conn));

  const char *error_message = NULL;
  int rc;
  ssh_key privkey = NULL;
  ssh_key pubkey = NULL;
  const struct sshcb_config *cfg = NULL;

  init_session_data(user);

  user->session = ssh_new();
  if(user->session == NULL)
    goto failure_connect;

  cfg = sshcb_get_config();

  if(cfg == NULL) {
    error_message = "Failed get sshcb_config";
    goto failure_connect;
  }

  user->port = cfg->client_port;
  rc = ssh_pki_import_pubkey_file(cfg->client_pubkey_path, &pubkey);
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

  rc = ssh_userauth_try_publickey(user->session, NULL, pubkey);
  if(rc != SSH_AUTH_SUCCESS)
    goto failure_connect;

  rc = ssh_pki_import_privkey_file(cfg->client_privkey_path, NULL, NULL, NULL, &privkey);

  if(rc != SSH_OK) goto failure_connect;

  rc = ssh_userauth_publickey(user->session, NULL, privkey);
  if(rc != SSH_AUTH_SUCCESS)
    goto failure_connect;

  ssh_key_free(privkey);
  ssh_key_free(pubkey);
  privkey = NULL;
  pubkey = NULL;

  rc = init_user_channels(user);
  if(rc == SSH_ERROR)
    goto failure_connect;

  ssh_set_blocking(user->session, 0);

  return user;

failure_connect:
  if(privkey != NULL) ssh_key_free(privkey);
  if(pubkey != NULL) ssh_key_free(pubkey);
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
  const struct sshcb_config *cfg = NULL;
  ssh_key privkey = NULL;

  init_session_data(server);

  server->session = ssh_new();
  if(server->session == NULL) {
    goto failure_init;
  }

  cfg = sshcb_get_config();

  if(cfg == NULL) {
    error_message = "Failed get sshcb_config";
    goto failure_init;
  }

  server->port = cfg->server_port;
  rc = ssh_pki_import_privkey_file(cfg->server_privkey_path, NULL, NULL, NULL, &privkey);
  if(rc != SSH_OK) {
    error_message = "Failed open server privkey\n";
    goto failure_init;
  }

  bind = bind_init(server, cfg, &privkey);

  if(bind == NULL) {
    error_message = "Bind failure";
    goto failure_init;
  }

  if(ssh_session_accept(server, bind) < 0) { 
    error_message = "Breake connection";
    goto failure_init;
  }

  ssh_bind_free(bind);
  bind = NULL;

//INFO:: ssh_bind_free will free binded privkey
  privkey = NULL;

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
  if(bind != NULL) ssh_bind_free(bind);
  if(privkey != NULL) ssh_key_free(privkey);
  log_error(server->session, error_message);
  ssh_conn_session_close(server);
  return NULL;
}

static ssh_bind bind_init(struct ssh_conn *server, const struct sshcb_config *cfg, ssh_key *privkey) {
  const char *error_message;
  int rc;
  ssh_bind bind;

  if(server == NULL) return NULL;
  if(cfg == NULL) return NULL; 


  bind = ssh_bind_new();
  if(bind == NULL) {
    error_message = "Failed ssh_bind_new";
    goto failure_bind;
  }

  rc = ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BINDADDR, cfg->bind_address);
  if (rc < 0) goto failure_bind;

  rc = ssh_bind_options_set(bind, SSH_BIND_OPTIONS_BINDPORT, &cfg->server_port);
  if (rc < 0) goto failure_bind;

  rc = ssh_bind_options_set(bind, SSH_BIND_OPTIONS_IMPORT_KEY, *privkey);
  if (rc < 0) goto failure_bind;

  if(ssh_bind_listen(bind) < 0)
    goto failure_bind;
  
  return bind;

failure_bind:
  error_message = ssh_get_error(bind);
  ssh_bind_free(bind);
  log_error(server->session, error_message);
  return NULL;
}

static int ssh_session_accept(struct ssh_conn *server, ssh_bind bind) {
  if(server == NULL) return -1;

  int rc;

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
  if(peer == NULL) return;

  while(peer->data.thread_state != IS_STOPPED && peer->data.thread_state != IS_IDLE) {
    cond_wait(&peer->data.cond, &peer->data.mutex);
  }
  mutex_destroy(&peer->data.mutex);
  cond_destroy(&peer->data.cond);

  log_info(peer->session, "close session");

  if(peer->session != NULL) {
    if(ssh_is_connected(peer->session)) {
      ssh_disconnect(peer->session);
    }
    close_channels(peer);
    ssh_free(peer->session);
    peer->session = NULL;    
  }

  free(peer); 
}

static void init_session_data(struct ssh_conn *peer) {
  peer->data.thread_state = IS_IDLE;
  peer->data.active_channels = 0;
  mutex_init(&peer->data.mutex);

  struct channel_context *ctx;
  for(int i = 0; i < MAX_CHANNELS; i++) {
    ctx = &peer->data.channels_data[i].ctx;
    memset(ctx, 0, sizeof(struct channel_context));
    ctx->state = STATE_IDLE;
    mutex_init(&ctx->mutex);
    cond_init(&ctx->cond);
  }
}
