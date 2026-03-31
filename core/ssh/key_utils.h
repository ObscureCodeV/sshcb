#ifndef KEY_UTILS_H
#define KEY_UTILS_H
#include "libssh/libssh.h"
#include <stdio.h>

int extract_pubkey_from_str(char *str, size_t size, ssh_key *key);
int extract_pubkey_from_file(FILE *fp, ssh_key *key);

#endif
