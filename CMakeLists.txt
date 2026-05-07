cmake_minimum_required(VERSION 3.14)
project(sshcb LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(TEST true)
  add_compile_definitions(TEST)
  set(SANITIZER_FLAGS 
    -fsanitize=address
    -fsanitize=undefined
    -fno-omit-frame-pointer
  )
  add_compile_options(${SANITIZER_FLAGS} -ggdb3 -O0)
  add_link_options(${SANITIZER_FLAGS})
  add_compile_options(--coverage)
  add_link_options(--coverage)
endif()

add_compile_options(-Wall -Wextra)

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(main)

if(TEST)
  add_subdirectory(test)
endif()

