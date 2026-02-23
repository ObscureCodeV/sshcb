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

  #ifndef SHUT_RDWR
    #define SHUT_RDWR SD_BOTH
  #endif

#else
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>

  static inline int socket_init(void) { return 0; }
  static inline int socket_close(int s) { return close(s); }
  static inline int socket_cleanup(void) { return 0; }
#endif

#endif

//libssh2 used libssh2_socket to simplest crossplatform development
