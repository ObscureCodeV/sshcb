#include "key_utils.h"
#include <stdio.h>
#include <libssh/libssh.h>
#include <string.h>

static const int MAX_LINE = 4096;

int extract_pubkey_from_file(FILE *fp, ssh_key *key) {
  char line[MAX_LINE];
  while(fgets(line, sizeof(line), fp) != NULL) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
    line[strcspn(line, "\r\n")] = '\0';
    return extract_pubkey_from_str(line, MAX_LINE, key);
  }
  return -1;
}

int extract_pubkey_from_str(char *str, size_t size, ssh_key *key) {

  char *key_type_end = strchr(str, ' ');
  size_t key_type_len = key_type_end - str;
  char key_type_str[key_type_len + 1];
  memcpy(key_type_str, str, key_type_len);
  key_type_str[key_type_len] = '\0';

  fprintf(stdout, "%s\n", key_type_str);

  enum ssh_keytypes_e type = ssh_key_type_from_name(key_type_str);
  if (type == SSH_KEYTYPE_UNKNOWN) {
    return SSH_ERROR;
  }

  char *base64_start = strchr(str, ' ');

  if(!base64_start)
    return SSH_ERROR;
  base64_start++;

  char *base64_end = strchr(base64_start, ' ');
  if(!base64_end)
    base64_end = str+strlen(str);

  size_t base64_len = base64_end - base64_start;
  char base64[base64_len + 1];
  memcpy(base64, base64_start, base64_len);
  base64[base64_len] = '\0';

//DEBUG:: test output
#ifdef TEST
  fprintf(stdout, "%s\n", base64);
  fprintf(stdout, "%zu\n", base64_len);
#endif

  return ssh_pki_import_pubkey_base64(base64, type, key);
}
