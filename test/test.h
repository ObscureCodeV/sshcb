#ifndef TEST_H
#define TEST_H

#define server_pubkey_file "server.pub"
#define knownhost_file "known_hosts"
#define auth_key_file "known_users"
#define user_pubkey_file "client.pub"

int test_server();
int test_client();

#endif
