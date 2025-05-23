#include "WinServer.hpp"

#include <vector>

#if defined(_WIN32) || defined(WIN32)

#include <WS2tcpip.h>
#include <ws2ipdef.h>
#include <WinSock2.h>

namespace Poulpe
{
  WinServer::WinServer(APIManager* APIManager):
    _api_manager(APIManager)
  {

  }

  WinServer::~WinServer()
  {
    ::closesocket(_ServSocket);
    ::closesocket(_Socket);
    ::WSACleanup();
  }

  void WinServer::close()
  {
    ::closesocket(_ServSocket);

    int status = ::WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("Error on WinServer close {}", status);
    }

    _Status = ServerStatus::NOT_RUNNING;
  }

  void WinServer::bind(std::string const& port)
  {
    int status = WSAStartup(MAKEWORD(2, 2), & _data);

    if (status != 0 ) {
      PLP_ERROR("WSAStartup failed with error: {}", status);
      WSACleanup();
    }

    PLP_TRACE("WSAStartup: {}", _data.szSystemStatus);

    addrinfo hints;
    addrinfo* servInfo{ nullptr }, *serv;
    memset(&hints, 0, sizeof hints);

    ZeroMemory(& hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = ::getaddrinfo(NULL, port.c_str(), &hints, &servInfo);

    if (status != 0) {
      PLP_ERROR("getaddrinfo failed with error: {} {}", gai_strerrorA(status), port);
      WSACleanup();
    }

    for (serv = servInfo; serv != nullptr; serv = serv->ai_next) {
      if (serv->ai_family == AF_INET) {
        _ServSocket = ::socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
        if (_ServSocket == static_cast<unsigned long long>(- 1)) {
          PLP_WARN("Socket creation failed {}", WSAGetLastError());
          continue;
        }
        break;
      } else if (serv->ai_family == AF_INET6) {
        _ServSocket = ::socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
        if (_ServSocket == static_cast<unsigned long long>(- 1)) {
          PLP_WARN("Socket creation failed {}", WSAGetLastError());
          continue;
        }
        break;
      }
    }

    if (_ServSocket == INVALID_SOCKET) {
      PLP_ERROR("ServerSocket failed with error: {}", ::gai_strerrorA(WSAGetLastError()));
      ::freeaddrinfo(servInfo);
      ::WSACleanup();
    }

    bool option{ true };
    int optionLen = sizeof (bool);
    setsockopt(_ServSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & option, optionLen);

    status = WSAGetLastError();

    if (status != 0) {
      PLP_ERROR("setsockopt failed {}", gai_strerrorA(WSAGetLastError()));
    }

    ::bind(_ServSocket, serv->ai_addr, static_cast<int>(serv->ai_addrlen));

    status = WSAGetLastError();

    if (status != 0) {
      PLP_ERROR("bind failed with error: {}", gai_strerrorA(status));
      ::closesocket(_ServSocket);
      ::WSACleanup();
    }

    char s[INET6_ADDRSTRLEN];

    if (serv->ai_addr->sa_family == AF_INET) {
      inet_ntop(serv->ai_family, &(((struct sockaddr_in*)serv)->sin_addr), s, sizeof s);
    } else {
      inet_ntop(serv->ai_family, &(((struct sockaddr_in6*)serv)->sin6_addr), s, sizeof s);
    }

    PLP_TRACE("Connecting to {}", s);

    ::freeaddrinfo(servInfo);
    _Status = ServerStatus::RUNNING;
  }

  void WinServer::listen()
  {
    ::listen(_ServSocket, 10);

    int status = ::WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("ServerSocket can't listen {}", gai_strerrorA(status));
      return;
    }

    bool done{ false };
    char s[INET6_ADDRSTRLEN];

    while (!done) {
      struct sockaddr_storage clientAddr;
      int sinSize = sizeof(clientAddr);
      SOCKET socket = ::accept(_ServSocket, (struct sockaddr *)&clientAddr, &sinSize);

      if (socket == INVALID_SOCKET) {
        perror("accept");

        PLP_ERROR("ServerSocket can't accept {}", ::gai_strerrorA(WSAGetLastError()));
        ::closesocket(_ServSocket);
        ::WSACleanup();
      } else {
        auto addr = (struct sockaddr*)&clientAddr;

        if (clientAddr.ss_family == AF_INET) {
          inet_ntop(clientAddr.ss_family, &(((struct sockaddr_in*)addr)->sin_addr), s, sizeof(s));
        } else {
          inet_ntop(clientAddr.ss_family, &(((struct sockaddr_in6*)addr)->sin6_addr), s, sizeof(s));
        }

        PLP_TRACE("server: got connection from {}", s);
        PLP_TRACE("Client connected");

        {
          std::lock_guard guard(_MutexSockets);
          _Socket = socket;
        }
        send("Connected to PoulpeEngine!\0");

        std::jthread readServer([this]() {
          read();
        });
        readServer.detach();
      }
    }
  }

  void WinServer::send(std::string message)
  {
    {
      std::lock_guard guard(_MutexSockets);
      int status = ::send(_Socket, message.data(), static_cast<int>(message.size()), 0);
      if (status == -1) {
        int err = WSAGetLastError();
        PLP_ERROR("Can't send data: [{}] {}", err, gai_strerrorA(err));
      }
      PLP_TRACE("sended: {}", message.data());
    }
  }

  void WinServer::read()
  {
    std::array<pollfd, 1> sockets;
    sockets[0].fd = _Socket;
    sockets[0].events = POLLIN;
    const int timeout{ 1000 };//1s
    bool hangup{ false };
    int status{ 0 };
    std::string message;

    while (!hangup) {
      status = WSAPoll(sockets.data(), sockets.size(), timeout);
      if (status == 0) {
        PLP_TRACE("poll timeout");
      } else if (status == SOCKET_ERROR) {
        int err = WSAGetLastError();
        PLP_WARN("Error will polling: [{}] {}", err, gai_strerrorA(err));
        hangup = true;
      } else {
        int events = sockets[0].revents & POLLIN;
        if (events) {
          PLP_TRACE("events poll for socket id: {}", sockets[0].fd);
          const int size = 10000;
          std::vector<char> buffer(size);//tmp
          int recvstatus{ 0 };

          message.clear();

          do {
            recvstatus = ::recv(_Socket, buffer.data(), size, 0);
            message.append(buffer.data());
            if (std::strcmp(buffer.data(), "\0") == 0) {
              recvstatus = -1;
            }
            PLP_WARN("status: {} msg: {}", recvstatus, message);
          } while (recvstatus > 0);
            if (message == "quit") hangup = true;
            _api_manager->received(message);
        } else {
          PLP_WARN("unexpected events poll for socket id: {}", sockets[0].fd);
          perror("send");
          hangup = true;
        }
      }
    }
  }
}

#endif
