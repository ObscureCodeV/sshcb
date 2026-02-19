#ifdef CHANNEL_H
#define CHANNEL_H

#include "data.h"

int add_channel(struct ConnectedData *conn);
int init_first_channel(struct ConnectedData *conn);
int remove_channel(struct ConnectedData *conn);

#endif
