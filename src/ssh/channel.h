#ifdef CHANNEL_H
#define CHANNEL_H

#include "data.h"

int open_channel(struct ConnectedData *conn, int idx);
int close_channel(struct ConnectedData *conn, int idx);
int close_all_channels(struct ConnectedData *conn);

#endif
