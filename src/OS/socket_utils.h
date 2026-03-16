#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")

  // ✅ Статическая переменная внутри функции = одна на программу
  static inline int socket_init(void) {
      static WSADATA wsaData;
      return WSAStartup(MAKEWORD(2,2), &wsaData);
  }

  static inline int socket_close(SOCKET s) {
      return closesocket(s);
  }

  static inline int socket_cleanup(void) {
      return WSACleanup();
  }

  #define cross_socket intptr_t

#else
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>

  static inline int socket_init(void) { return 0; }
  static inline int socket_close(int s) { return close(s); }
  static inline int socket_cleanup(void) { return 0; }

  #define cross_socket intptr_t
#endif

  int select_timeout(cross_socket sock, fd_set fd_in, fd_set fd_out, int timeout_ms);

#endif
