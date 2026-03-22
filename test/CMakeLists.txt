add_subdirectory(test-res)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

add_executable(test-server server.c test.c)
target_include_directories(test-server PRIVATE
  ../core
  ${LIBSSH_INCLUDE_DIRS}
)
target_link_libraries(test-server PUBLIC core)
target_link_libraries(test-server PUBLIC PkgConfig::LIBSSH)
target_compile_options(test-server PUBLIC ${LIBSSH_CFLAGS_OTHER})
target_include_directories(test-server PUBLIC ${LIBSSH_INCLUDE_DIRS})

add_executable(test-client client.c test.c)
target_include_directories(test-client PRIVATE
  ../core
  ${LIBSSH_INCLUDE_DIRS}
)
target_link_libraries(test-client PUBLIC core)
target_link_libraries(test-client PUBLIC PkgConfig::LIBSSH)
target_compile_options(test-client PUBLIC ${LIBSSH_CFLAGS_OTHER})
target_include_directories(test-client PUBLIC ${LIBSSH_INCLUDE_DIRS})


