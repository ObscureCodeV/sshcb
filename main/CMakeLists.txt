find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)

add_executable(app "main.c")
target_include_directories(app PRIVATE
    ../core
    ../common
      ${LIBSSH_INCLUDE_DIRS}
    )
target_link_libraries(app PRIVATE core)
target_link_libraries(app PRIVATE common)

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
