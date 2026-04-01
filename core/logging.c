#include "logging.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

void log_init(const char *app_name) {
  return;
}

void log_close() {
  return;
}

void log_info(ssh_session session, const char *format, ...) {
  char message[1024];
  va_list args;

  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  fprintf(stdout, "[session::%p] %s", (void*)session, message);
  fprintf(stdout, "\n");
}

void log_error(ssh_session session, const char *format, ...) {

  char message[1024];
  va_list args;

  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  fprintf(stdout, "[session::%p] %s", (void*)session, message);
  fprintf(stdout, "\n");
}
