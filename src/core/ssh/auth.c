#include "auth.h"
#include "config.h"
#include "key_utils.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdio.h>
#include <string.h>


int verify_host(ssh_session session) {
  const char *error_message = NULL;
  ssh_key srv_pubkey = NULL;
  
  if(ssh_get_server_publickey(session, &srv_pubkey) < 0) {
    error_message = ssh_get_error(session);
    goto failure_check_host;
  }
 
  unsigned char *hash = NULL;
  size_t hlen; 
  if(ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen) < 0) {
    goto failure_check_host;
  }
  
  enum ssh_known_hosts_e state;

  state = ssh_session_is_known_server(session);

  switch(state) {
    case SSH_KNOWN_HOSTS_CHANGED:
      error_message = "";
      goto failure_check_host;

    case SSH_KNOWN_HOSTS_OTHER:
      error_message = "";
      goto failure_check_host;

    case SSH_KNOWN_HOSTS_NOT_FOUND:
      error_message = "";
      goto failure_check_host;

    case SSH_KNOWN_HOSTS_UNKNOWN:
      error_message = "";
      goto failure_check_host;

    case SSH_KNOWN_HOSTS_ERROR:
      error_message = "";
      goto failure_check_host;
  }

  return 0;

failure_check_host:
  //TODO change logging
  if(error_message != NULL) fprintf(stderr, "%s\n", error_message);
  return -1;
}

#define MAX_LINE 4096

int verify_user(ssh_session session, const char *user, struct ssh_key_struct *pubkey, char signature_state, void *userdata) {

  const char *error_message;

  FILE *fp = fopen(AUTH_KEYS_FILE, "r");
  if (fp == NULL) {
    error_message = "Cannot open authfile";
    goto failure_check_user;
  }

  int rc = 0;
  ssh_key candidate;

  do {
    candidate = NULL;

    rc = exract_pubkey_from_file(fp, candidate);

    if (rc == SSH_OK) {
        if (ssh_key_cmp(pubkey, candidate, SSH_KEY_CMP_PUBLIC) == 0) {
            ssh_key_free(candidate);
            fclose(fp);
            return SSH_AUTH_SUCCESS;
        }
        ssh_key_free(candidate);
    }
  } while (rc != -1);

  error_message = "User public key not found in authorized_keys";
  fclose(fp);

failure_check_user:
  //TODO change logging
  if(error_message != NULL) fprintf(stderr, "%s\n", error_message);
  return SSH_AUTH_DENIED;
}
