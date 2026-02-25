#ifdef AUTH_H
#define AUTH_H

#include <libssh2.h>

int verify_host(LIBSSH2_SESSION *session, const char *host);

#endif
