add_library(cli STATIC
  cli_utils.h
  cli_utils.c
)

target_include_directories(cli PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../daemon
)

target_link_libraries(cli PRIVATE daemon)
