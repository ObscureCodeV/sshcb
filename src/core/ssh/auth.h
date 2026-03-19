#ifndef VERIFY_AUTH_H
#define VERIFY_AUTH_H

#include <libssh/libssh.h>

int verify_host(ssh_session session);
int verify_user(ssh_session session, const char *user, struct ssh_key_struct *pubkey, char signature_state, void *userdata);

#endif
