#ifndef CONFIG_H
#define CONFIG_H

const struct sshcb_config* sshcb_get_config(void);

struct sshcb_config {
  const char *known_host_path;
  const char *auth_key_path;

  const char *client_pubkey_path;
  const char *client_privkey_path;

  const char *server_pubkey_path;
  const char *server_privkey_path;

  int server_port;
  int client_port;

  const char *bind_address;
};

#endif
