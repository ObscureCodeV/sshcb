
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
