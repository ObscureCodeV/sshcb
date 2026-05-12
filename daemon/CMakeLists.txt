add_library(daemon STATIC
  daemon_wrapper.h
  daemon_wrapper.c
  daemon_utils.h
  daemon_utils.c
)

target_include_directories(daemon PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../core
)

target_link_libraries(daemon PRIVATE core)
