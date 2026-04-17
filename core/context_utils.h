#include "ssh/data.h"

void write_data(struct channel_pair *pair, const void *buf, const size_t len);

void read_data(struct channel_pair *pair, char **buf, size_t *len);
