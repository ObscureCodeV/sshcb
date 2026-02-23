cmake_minimum_required(VERSION 3.14)
project(my_ssh_app LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    libssh2
    GIT_REPOSITORY https://github.com/libssh2/libssh2.git
    GIT_TAG        libssh2-1.11.1
)

FetchContent_MakeAvailable(libssh2)

set(SOURCES
    src/main.c
    src/ssh/session.c
    src/ssh/channel.c
)

add_executable(app ${SOURCES})

target_include_directories(app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(app PRIVATE
    libssh2_static
)

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
