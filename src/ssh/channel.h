#ifdef CHANNEL_H
#define CHANNEL_H

#include "data.h"

int open_channel(struct ConnectedData *conn, int idx);
int close_channel(struct ConnectedData *conn, int idx);
int close_all_channels(struct ConnectedData *conn);
int read_channel(struct ConnecteData *conn, int idx);
int write_channel(struct ConnectedData *conn, int channel_idx, int context_idx);

#endif
