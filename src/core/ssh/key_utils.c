#include "key_utils.h"
#include <stdio.h>
#include <libssh/libssh.h>
#include <string.h>

int exract_pubkey_from_str(char *str, size_t size, ssh_key key) {
  return ssh_pki_import_pubkey_base64(str, size, &key);
}

static const int MAX_LINE = 4096;

int exract_pubkey_from_file(FILE *fp, ssh_key key) {
  char line[MAX_LINE];
  int rc;
  while(fgets(line, sizeof(line), fp) != NULL) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
    line[strcspn(line, "\r\n")] = '\0';
    return exract_pubkey_from_str(line, MAX_LINE, key);
  }
  return -1;
}
