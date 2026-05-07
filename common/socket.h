#ifndef IPC_SOCKET_H
#define IPC_SOCKET_H

#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
  typedef SOCKET socket_t;
  #define INVALID_SOCKET_VAL INVALID_SOCKET
  #define SOCKET_PATH "\\\\.\\pipe\\sshcb"

#else
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <unistd.h>
  typedef int socket_t;
  #define INVALID_SOCKET_VAL -1
  #define SOCKET_PATH "/tmp/sshcb.sock"

#endif
  socket_t create_server_socket(const char *path);
  socket_t create_client_socket(const char *path);
  int send_message(socket_t sock, const void *msg, size_t len);
  int recv_message(socket_t sock, void *buf, size_t buf_size, size_t *out_len);
  void close_socket(socket_t sock);
  int socket_startup();
  void socket_cleanup();
#endif
