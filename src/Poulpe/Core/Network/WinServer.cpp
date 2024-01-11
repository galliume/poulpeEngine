#include "WinServer.hpp"

#include <vector>

#if defined(_WIN32) || defined(WIN32)

#include <WS2tcpip.h>
#include <ws2ipdef.h>

namespace Poulpe
{
  WinServer::~WinServer()
  {
    ::closesocket(m_ServSocket);

    for (auto& socket : m_Sockets) {
      ::closesocket(socket);
    }
    ::WSACleanup();
  }

  void WinServer::close()
  {
    ::closesocket(m_ServSocket);

    int status = ::WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("Error on WinServer close {}", status);
    }

    m_Status = ServerStatus::NOT_RUNNING;
  }

  void WinServer::bind(std::string const& port)
  {
    int status = WSAStartup(MAKEWORD(2, 2), & m_Data);

    if (status != 0 ) {
      PLP_ERROR("WSAStartup failed with error: {}", status);
      WSACleanup();
    }

    PLP_TRACE("WSAStartup: {}", m_Data.szSystemStatus);

    addrinfo hints;
    addrinfo* servInfo{ nullptr }, *p;
    memset(&hints, 0, sizeof hints);

    ZeroMemory(& hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
    status = ::getaddrinfo(NULL, port.c_str(), &hints, &servInfo);
    #pragma clang diagnostic pop

    if (status != 0) {
      PLP_ERROR("getaddrinfo failed with error: {} {}", gai_strerrorA(status), port);
      WSACleanup();
    }

    char ipstr[INET6_ADDRSTRLEN];

    for (p = servInfo; p != nullptr; p = p->ai_next) {
      void *addr;
      std::string ipver;

      if (p->ai_family == AF_INET6) {
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wcast-align"
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        #pragma clang diagnostic pop
        addr = &(ipv6->sin6_addr);
      }

      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      PLP_TRACE("Server IP: {}", ipstr);
    }

    m_ServSocket = ::socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);

    if (m_ServSocket == INVALID_SOCKET) {
      PLP_ERROR("ServerSocket failed with error: {}", ::gai_strerrorA(WSAGetLastError()));
      ::freeaddrinfo(servInfo);
      ::WSACleanup();
    }

    bool option{ true };
    int optionLen = sizeof (bool);
    setsockopt(m_ServSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & option, optionLen);

    status = WSAGetLastError();

    if (status != 0) {
      PLP_ERROR("setsockopt failed {}", gai_strerrorA(WSAGetLastError()));
    }

    ::bind(m_ServSocket, servInfo->ai_addr, static_cast<int>(servInfo->ai_addrlen));

    status = WSAGetLastError();

    if (status != 0) {
      PLP_ERROR("bind failed with error: {}", gai_strerrorA(status));
      ::freeaddrinfo(servInfo);
      ::closesocket(m_ServSocket);
      ::WSACleanup();
    }

    ::freeaddrinfo(servInfo);
  }

  void WinServer::listen()
  {
    ::listen(m_ServSocket, 10);

    int status = ::WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("ServerSocket can't listen {}", gai_strerrorA(status));
      return;
    }

    m_Status = ServerStatus::RUNNING;
    PLP_TRACE("Server running");

    while (true) {
      SOCKET socket = ::accept(m_ServSocket, nullptr, nullptr);
      
      if (socket == INVALID_SOCKET)
      {
        PLP_ERROR("ServerSocket can't accept {}", ::gai_strerrorA(WSAGetLastError()));
        ::closesocket(m_ServSocket);
        ::WSACleanup();
      } else {
        PLP_TRACE("Client connected");
        {
          std::lock_guard guard(m_MutexSockets);
          m_Sockets.emplace_back(socket);
        }
        send("Connected to PoulpeEngine!");
      }
    }
  }

  void WinServer::send(std::string message)
  {
    {
      std::lock_guard guard(m_MutexSockets);

      for (auto& socket : m_Sockets) {
        ::send(socket, message.data(), static_cast<int>(message.size()), 0);
      }
    }
  }

  void WinServer::read()
  {
    char recvbuf[512];
    int recvbuflen = 512;

    int status{ 0 };

    {
      std::lock_guard guard(m_MutexSockets);

      for (auto& socket : m_Sockets) {
        do {
          status = ::recv(socket, recvbuf, recvbuflen, 0);
          PLP_TRACE("bytes received {}", status);
          PLP_TRACE("received {}", recvbuf);
        } while (status > 0);

        closesocket(socket);
        WSACleanup();
      }
    }
  }
}

#endif
