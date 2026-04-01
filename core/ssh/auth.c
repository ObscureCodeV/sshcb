#include "auth.h"
#include "key_utils.h"
#include "../logging.h"
#include <libssh/libssh.h>
#include <libssh/server.h>
#include <stdio.h>
#include <string.h>

int verify_host(ssh_session session) {
  const char *error_message = NULL;
  ssh_key srv_pubkey = NULL;
  unsigned char *hash = NULL;
  size_t hlen;

//DEBUG
  fprintf(stdout, "%s\n", "Inner to verify_host");
  
  if(ssh_get_server_publickey(session, &srv_pubkey) < 0) {
    error_message = ssh_get_error(session);
    goto failure_check_host;
  }

 log_info(session, "Success get server publickey");
 
  if(ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen) < 0) {
    goto failure_check_host;
  }
  
  enum ssh_known_hosts_e state;

  state = ssh_session_is_known_server(session);
  if(state == SSH_KNOWN_HOSTS_OK) {
    log_info(session, "success verify host");
    ssh_clean_pubkey_hash(&hash);
    return 0;
  }

  switch(state) {
    case SSH_KNOWN_HOSTS_CHANGED:
      error_message = "";
      break;

    case SSH_KNOWN_HOSTS_OTHER:
      error_message = "";
      break;

    case SSH_KNOWN_HOSTS_NOT_FOUND:
      error_message = "";
      break;

    case SSH_KNOWN_HOSTS_UNKNOWN:
      error_message = "";
      break;

    case SSH_KNOWN_HOSTS_ERROR:
      error_message = "";
      break;
  }

failure_check_host:
  if(error_message != NULL) log_error(session, error_message);
  if(hash != NULL) ssh_clean_pubkey_hash(&hash);
  return -1;
}

#define MAX_LINE 4096

int verify_user(ssh_session session, const char *user, struct ssh_key_struct *pubkey, char signature_state, void *userdata) {

  fprintf(stdout, "%s\n", "Inner to verify_user");

  const char *error_message;
  FILE *fp = NULL;

#ifdef TEST
  fp = fopen("test-res/known_users", "r");
#endif
  if (fp == NULL) {
    error_message = "Cannot open authfile";
    goto failure_check_user;
  }

  int rc = 0;
  ssh_key candidate;

  do {
    candidate = NULL;

    rc = extract_pubkey_from_file(fp, &candidate);

    if (rc == SSH_OK) {
        if (ssh_key_cmp(pubkey, candidate, SSH_KEY_CMP_PUBLIC) == 0) {
            log_info(session, "valid user key");
            if(signature_state) {
              if(signature_state != SSH_PUBLICKEY_STATE_VALID)
                break;
            }
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
  if(error_message != NULL) log_error(session, error_message);
  return SSH_AUTH_DENIED;
}
