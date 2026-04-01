#ifndef LOG_H
#define LOG_H

#include <libssh/libssh.h>

void log_init(const char *app_name);
void log_error(ssh_session session, const char *format, ...);
void log_info(ssh_session session, const char *format, ...);
void log_close();

#endif
