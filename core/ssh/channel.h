#ifdef CHANNEL_H
#define CHANNEL_H

#include "data.h"

int init_channels(ssh_session *session, ssh_channel *channels, int NumChannels);
int shutdown_channels(ssh_session *session, ssh_channel *channels, int NumChannels);
int open_channel(ssh_channel *channel);
int open_channel(struct ConnectedData *conn, int idx);
int close_channel(ssh_channel *channel);
int write_channel(channel *channel, struct ChannelContext *context);
int read_channel(channel *channel, struct ChannelContext *context);

#endif
