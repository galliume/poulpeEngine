#include "WinServer.hpp"

#include <vector>

#if defined(_WIN32) || defined(WIN32)

#include <WS2tcpip.h>

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
      PLP_ERROR("getaddrinfo failed with error: {}", status);
      WSACleanup();
    }

    PLP_TRACE("WSAStartup: {}", m_Data.szSystemStatus);

    addrinfo hints;
    addrinfo* results{ nullptr };

    ZeroMemory(& hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    status = ::getaddrinfo("localhost", port.c_str(), &hints, &results);

    if (status != 0) {
      PLP_ERROR("getaddrinfo failed with error: {} {}", status, port);
      WSACleanup();
    }

    m_ServSocket = ::socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    if (m_ServSocket == INVALID_SOCKET) {
      PLP_ERROR("ServerSocket failed with error: {}", ::WSAGetLastError());
      ::freeaddrinfo(results);
      ::WSACleanup();
    }

    ::bind(m_ServSocket, results->ai_addr, (int)results->ai_addrlen);

    status = WSAGetLastError();

    if (status != 0) {
      PLP_ERROR("bind failed with error: {}", ::WSAGetLastError());
      ::freeaddrinfo(results);
      ::closesocket(m_ServSocket);
      ::WSACleanup();
    }

    bool option{ false };
    int optionLen = sizeof (bool);
    setsockopt(m_ServSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & option, optionLen);

    status = WSAGetLastError();

    if (status != 0) {
      PLP_ERROR("setsockopt failed {}", WSAGetLastError());
    }

    ::freeaddrinfo(results);
  }

  void WinServer::listen()
  {
    ::listen(m_ServSocket, SOMAXCONN);

    int status = ::WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("ServerSocket can't listen {}", status);
      return;
    }

    m_Status = ServerStatus::RUNNING;
    PLP_TRACE("Server running");

    while (true) {
      SOCKET socket = ::accept(m_ServSocket, nullptr, nullptr);
      
      if (socket == INVALID_SOCKET)
      {
        PLP_ERROR("ServerSocket can't accept {}", ::WSAGetLastError());
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
