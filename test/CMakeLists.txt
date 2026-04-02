add_subdirectory(test-res)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

set(TEST_RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test-res)

function(add_test_executable name)
  add_executable(${name} ${ARGN})
  target_include_directories(${name} PRIVATE ../core ${LIBSSH_INCLUDE_DIRS})
  target_link_libraries(${name} PRIVATE core PkgConfig::LIBSSH)
  target_compile_options(${name} PRIVATE ${LIBSSH_CFLAGS_OTHER})
endfunction()

add_test_executable(test-server server.c test.c)
add_test_executable(test-client client.c test.c)

add_test(NAME env_server COMMAND test-server)
set_tests_properties(env_server PROPERTIES
  ENVIRONMENT_MODIFICATION "SSHCB_AUTHORIZED_KEYS=${TEST_RES_DIR}/known_users;SSHCB_PORT=2456;SSHCB_SERVER_PUBKEY_FILE=${TEST_RES_DIR}/server.pub;SSHCB_SERVER_PRIVKEY_FILE=${TEST_RES_DIR}/server;SSHCB_SERVER_INTERFACE_IP=127.0.0.1"
)

add_test(NAME env_client COMMAND test-client)
set_tests_properties(env_client PROPERTIES
  ENVIRONMENT_MODIFICATION "SSHCB_KNOWN_HOSTS=${TEST_RES_DIR}/known_hosts;SSHCB_PORT=2456;SSHCB_CLIENT_PUBKEY_FILE=${TEST_RES_DIR}/client.pub;SSHCB_CLIENT_PRIVKEY_FILE=${TEST_RES_DIR}/client"
)
