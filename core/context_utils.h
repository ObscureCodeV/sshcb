#include "ssh/data.h"

void write_data(struct ssh_conn *conn, int channel_idx, const void *buf, const size_t len);
size_t read_data(struct ssh_conn *conn, int channel_idx, char *buf);
void clear_readed(struct ssh_conn *conn, int channel_idx);
