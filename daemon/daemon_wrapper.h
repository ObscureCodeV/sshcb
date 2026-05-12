#ifndef DAEMON_WRAPPER_H
#define DAEMON_WRAPPER_H

int daemon_is_running(void);
int daemon_run(int (*main_func)(void));

#endif
