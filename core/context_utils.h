#include "ssh/data.h"

int write_data(struct ssh_conn *conn, int channel_idx, const void *buf, const size_t len);
int read_data(struct ssh_conn *conn, int channel_idx, char *buf);
void clear(struct ssh_conn *conn, int channel_idx);
