cmake_minimum_required(VERSION 3.14)
project(my_ssh_app LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

#set(TEST true)

#if(TEST)
#  add_subdirectory(test-res)

set(SOURCES
    src/main/main.c
    src/core/ssh/key_utils.c
    src/core/ssh/auth.c
    src/core/ssh/session.c
    src/core/ssh/channel.c
    src/core/OS/socket_utils.c
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

add_executable(app ${SOURCES})

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${LIBSSH_INCLUDE_DIRS}
)

target_link_libraries(app PRIVATE PkgConfig::LIBSSH)
target_compile_options(app PRIVATE ${LIBSSH_CFLAGS_OTHER})
target_include_directories(app PRIVATE ${LIBSSH_INCLUDE_DIRS})

if(WIN32)
    target_link_libraries(app PRIVATE 
        ws2_32 
        crypt32 
        bcrypt
        ncrypt
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(app PRIVATE 
        pthread 
        ${CMAKE_DL_LIBS}
    )
elseif(APPLE)
    target_link_libraries(app PRIVATE 
        "-framework Security" 
        "-framework CoreFoundation"
    )
endif()
