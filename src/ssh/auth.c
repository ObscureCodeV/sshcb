#include "auth.h"
#include <libssh2.h>
#include <stdio.h>

#define HOST_FILE "Futute config setting"
#define TYPE_MASK (LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW)

int verify_host(LIBSSH2_SESSION *session, const char *host) {
  char *error_message = NULL;

  if (session == NULL || host == NULL) {
    error_message = "Invalid arguments to check_host";
    goto failure_check_host;
  }

  LIBSSH2_KNOWNHOSTS *kh = libssh2_knownhost_init(session);
  if (kh == NULL) {
    error_message = "Failed to init known_hosts";
    goto failure_check_host;
  }
  
  int num_knownhost = libssh2_knownhost_readfile(kh, HOST_FILE, LIBSSH2_KNOWNHOST_FILE_OPENSSH);

  if(num_knownhost <= 0) {
    error_message = "";
    goto failure_check_host;
  }

  size_t keylen;
  int keytype;
  const char *key = libssh2_session_hostkey(session, &keylen, &keytype);

  if(key == NULL) {
    error_message="";
    goto failure_check_host;
  }
  
  struct libssh2_knownhost *knownhost = NULL;
  int rc = libssh2_knownhost_check(kh, host, key, keylen, TYPE_MASK, &knownhost);

  if (rc == LIBSSH2_KNOWNHOST_CHECK_MATCH) {
    libssh2_knownhost_free(kh);
    return 0;
  } 
  else if (rc == LIBSSH2_KNOWNHOST_CHECK_NOT_FOUND) {
    error_message="";
  } 
  else if (rc == LIBSSH2_KNOWNHOST_CHECK_MISMATCH) {
    error_message = "";
  }

failure_check_host:
  //TODO change logging
  if(kh) libssh2_knownhost_free(kh);
  if(error_message) fprintf(stderr, "%s\n", error_message);
  return -1;
}
