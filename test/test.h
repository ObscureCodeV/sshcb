#ifndef TEST_H
#define TEST_H

#define server_pubkey_file "test-res/server.pub"
#define knownhost_file "test-res/known_hosts"
#define auth_key_file "test-res/known_users"
#define user_pubkey_file "test-res/client.pub"
#define server_privkey_file "test-res/server"

int test_server();
int test_client();

#endif
