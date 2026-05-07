#include "socket.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
  int socket_startup() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
      fprintf(stderr, "WSAStartup failed\n");
      return 1;
    }
    return 0;
  }

  void socket_cleanup() {
    WSACleanup();
  }

  socket_t create_server_socket(const char *path) {
    HANDLE pipe = CreateNamedPipeA(
        path,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        4096, 4096, 0, NULL
    );
    return (socket_t)pipe;
  } 
  
  socket_t create_client_socket(const char *path) {
    HANDLE pipe = CreateFileA(
      path,
      GENERIC_READ | GENERIC_WRITE,
      0, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, NULL
    );
    return (pipe == INVALID_HANDLE_VALUE) ? INVALID_SOCKET_VAL : (socket_t)pipe;
    
  int send_message(socket_t sock, const void *msg, size_t len) {
    DWORD written;
    HANDLE pipe = (HANDLE)sock;
    if (!WriteFile(pipe, msg, len, &written, NULL)) {
      return -1;
    }
    return (written == len) ? 0 : -1;
  }

  int recv_message(socket_t sock, void *buf, size_t buf_size, size_t *out_len) {
    DWORD read;
    HANDLE pipe = (HANDLE)sock;
    if (!ReadFile(pipe, buf, buf_size, &read, NULL)) {
      return -1;
    }
    if (out_len) *out_len = read;
    return 0;
  }
  
  void close_socket(socket_t sock) {
    HANDLE pipe = (HANDLE)sock;
    CloseHandle(pipe);
  }

#else
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>

  int socket_startup() {
    return 0;
  }

  void socket_cleanup() {
    return;
  }
  
  socket_t create_server_socket(const char *path) {
    unlink(path);
    socket_t sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL) {
      perror("socket");
      return INVALID_SOCKET_VAL;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
      perror("bind");
      close(sock);
      return INVALID_SOCKET_VAL;
    }
    if (listen(sock, 5) == -1) {
      perror("listen");
      close(sock);
      return INVALID_SOCKET_VAL;
    }
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    return sock;
  }
  
  socket_t create_client_socket(const char *path) {
    socket_t sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL) {
      perror("socket");
      return INVALID_SOCKET_VAL;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
      close(sock);
      return INVALID_SOCKET_VAL;
    }
    return sock;
  }
  
  int send_message(socket_t sock, const void *msg, size_t len) {
    ssize_t sent = write(sock, msg, len);
    if (sent < 0 || (size_t)sent != len) {
      return -1;
    }
    return 0;
  }
  
  int recv_message(socket_t sock, void *buf, size_t buf_size, size_t *out_len) {
    ssize_t received = read(sock, buf, buf_size);
    if (received < 0) {
      return -1;
    }
    if (out_len) *out_len = received;
    return 0;
  }
  
  void close_socket(socket_t sock) {
    close(sock);
  }

#endif
