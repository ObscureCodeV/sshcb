find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

add_executable(test-server server.c test.c)
target_include_directories(test-server PRIVATE
  ../core
  ${LIBSSH_INCLUDE_DIRS}
)
target_link_libraries(test-server PRIVATE core)
target_link_libraries(test-server PRIVATE PkgConfig::LIBSSH)
target_compile_options(test-server PRIVATE ${LIBSSH_CFLAGS_OTHER})
target_include_directories(test-server PRIVATE ${LIBSSH_INCLUDE_DIRS})

add_executable(test-client client.c test.c)
target_include_directories(test-client PRIVATE
  ../core
  ${LIBSSH_INCLUDE_DIRS}
)
target_link_libraries(test-client PRIVATE core)
target_link_libraries(test-client PRIVATE PkgConfig::LIBSSH)
target_compile_options(test-client PRIVATE ${LIBSSH_CFLAGS_OTHER})
target_include_directories(test-client PRIVATE ${LIBSSH_INCLUDE_DIRS})


