#ifndef CHANNEL_H
#define CHANNEL_H

#include "data.h"

int init_user_channels(struct ssh_conn *peer);
void close_channels(struct ssh_conn *peer);
int send_data(struct ssh_conn *conn, int channel_idx);

//INFO:: for server callback
ssh_channel server_channel_open(ssh_session session, void *userdata);
void server_channel_close(ssh_session session, ssh_channel channel, void *userdata);

#endif
