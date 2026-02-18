cmake_minimum_required(VERSION 3.10)
project(my_ssh_app LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBSSH2 REQUIRED libssh2)

set(SOURCES
    src/main.c
    src/ssh/session.c
)

add_executable(app ${SOURCES})

target_include_directories(app PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${LIBSSH2_INCLUDE_DIRS}
)

target_link_libraries(app PRIVATE 
    ${LIBSSH2_LIBRARIES}
)

target_compile_options(app PRIVATE ${LIBSSH2_CFLAGS_OTHER})
