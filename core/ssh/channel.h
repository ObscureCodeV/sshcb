#ifndef CHANNEL_H
#define CHANNEL_H

#include "data.h"

int check_recv_success(const struct ChannelContext *context);
int init_user_channels(struct ssh_conn *peer, const char *host);
int init_server_channels(struct ssh_conn *peer);
void close_channels(struct ssh_conn *peer);
int send_data(ssh_channel *channel, struct ChannelContext *context);

#endif
