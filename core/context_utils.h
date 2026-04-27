#include "ssh/data.h"

void write_data(struct channel_pair *pair, const void *buf, const size_t len);
size_t read_data(struct channel_pair *pair, char *buf);
void clear_readed(struct channel_pair *pair);
