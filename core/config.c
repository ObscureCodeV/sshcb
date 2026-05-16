#include "config.h"
#include <stdlib.h>

static struct sshcb_config cfg;
static int initialized = 0;

const struct sshcb_config* sshcb_get_config(void) {
  if(initialized)
    return &cfg;

  cfg.known_host_path = getenv("SSHCB_KNOWNHOST");
  cfg.auth_key_path = getenv("SSHCB_AUTHORIZED_KEYS");

  int port = atoi(getenv("SSHCB_PORT"));
  cfg.server_port = port;
  cfg.client_port = port;

  cfg.client_pubkey_path = getenv("SSHCB_CLIENT_PUBKEY_FILE"); 
  cfg.client_privkey_path = getenv("SSHCB_CLIENT_PRIVKEY_FILE");

  cfg.server_pubkey_path = getenv("SSHCB_SERVER_PUBKEY_FILE");
  cfg.server_privkey_path = getenv("SSHCB_SERVER_PRIVKEY_FILE");
  
  initialized = 1;

  return &cfg;
}
