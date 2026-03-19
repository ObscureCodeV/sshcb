cmake_minimum_required(VERSION 3.14)
project(sshcb LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(TEST true)

if(TEST)
  add_subdirectory(test)
  add_subdirectory(test-res)
endif()

add_subdirectory(core)
add_subdirectory(main)

