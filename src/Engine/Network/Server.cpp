module;
#if defined(_WIN64)
  #include <ws2tcpip.h>
  #include <ws2ipdef.h>
  #include <winsock2.h>

  #else
  //@todo linux impl
#endif

module Engine.Network.Server;

import std;

namespace Poulpe
{
  #if defined(_WIN64)
    WinServer::~WinServer()
    {
      ::closesocket(_servSocket);
      ::closesocket(_socket);
      ::WSACleanup();
    }

    void WinServer::close()
    {
      ::closesocket(_servSocket);

      int status = ::WSAGetLastError();

      if (0 != status) {
        Logger::error("Error on WinServer close {}", status);
      }

      _status = ServerStatus::NOT_RUNNING;
    }

    void WinServer::bind(std::string const& port)
    {
      int status = WSAStartup(MAKEWORD(2, 2), & _data);

      if (status != 0 ) {
        Logger::error("WSAStartup failed with error: {}", status);
        WSACleanup();
      }

      Logger::trace("WSAStartup: {}", _data.szSystemStatus);

      addrinfo hints;
      addrinfo* servInfo{ nullptr }, *serv;
      memset(&hints, 0, sizeof hints);

      ZeroMemory(& hints, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;

      status = ::getaddrinfo(nullptr, port.c_str(), &hints, &servInfo);

      if (status != 0) {
        Logger::error("getaddrinfo failed with error: {} {}", gai_strerrorA(status), port);
        WSACleanup();
      }

      for (serv = servInfo; serv != nullptr; serv = serv->ai_next) {
        if (serv->ai_family == AF_INET) {
          _servSocket = ::socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
          if (_servSocket == static_cast<unsigned long long>(- 1)) {
            Logger::warn("Socket creation failed {}", WSAGetLastError());
            continue;
          }
          break;
        } else if (serv->ai_family == AF_INET6) {
          _servSocket = ::socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
          if (_servSocket == static_cast<unsigned long long>(- 1)) {
            Logger::warn("Socket creation failed {}", WSAGetLastError());
            continue;
          }
          break;
        }
      }

      if (_servSocket == INVALID_SOCKET) {
        Logger::error("ServerSocket failed with error: {}", ::gai_strerrorA(WSAGetLastError()));
        ::freeaddrinfo(servInfo);
        ::WSACleanup();
      }

      bool option{ true };
      int optionLen = sizeof (bool);
      setsockopt(_servSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & option, optionLen);

      status = WSAGetLastError();

      if (status != 0) {
        Logger::error("setsockopt failed {}", gai_strerrorA(WSAGetLastError()));
      }

      ::bind(_servSocket, serv->ai_addr, static_cast<int>(serv->ai_addrlen));

      status = WSAGetLastError();

      if (status != 0) {
        Logger::error("bind failed with error: {}", gai_strerrorA(status));
        ::closesocket(_servSocket);
        ::WSACleanup();
      }

      char s[INET6_ADDRSTRLEN];

      if (serv->ai_addr->sa_family == AF_INET) {
        inet_ntop(serv->ai_family, &(((struct sockaddr_in*)serv)->sin_addr), s, sizeof s);
      } else {
        inet_ntop(serv->ai_family, &(((struct sockaddr_in6*)serv)->sin6_addr), s, sizeof s);
      }

      Logger::trace("Connecting to {}", s);

      ::freeaddrinfo(servInfo);
      _status = ServerStatus::RUNNING;
    }

    void WinServer::listen()
    {
      ::listen(_servSocket, 10);

      int status = ::WSAGetLastError();

      if (0 != status) {
        Logger::error("ServerSocket can't listen {}", gai_strerrorA(status));
        return;
      }

      bool done{ false };
      char s[INET6_ADDRSTRLEN];

      while (!done) {
        struct sockaddr_storage clientAddr;
        int sinSize = sizeof(clientAddr);
        SOCKET socket = ::accept(_servSocket, (struct sockaddr *)&clientAddr, &sinSize);

        if (socket == INVALID_SOCKET) {
          perror("accept");

          Logger::error("ServerSocket can't accept {}", ::gai_strerrorA(WSAGetLastError()));
          ::closesocket(_servSocket);
          ::WSACleanup();
        } else {
          auto addr = (struct sockaddr*)&clientAddr;

          if (clientAddr.ss_family == AF_INET) {
            auto *addr_in = reinterpret_cast<struct sockaddr_in*>(addr);
            inet_ntop(clientAddr.ss_family, &(addr_in->sin_addr), s, sizeof(s));
          } else {
            auto *addr_in6 = reinterpret_cast<struct sockaddr_in6*>(addr);
            inet_ntop(clientAddr.ss_family, &(addr_in6->sin6_addr), s, sizeof(s));
          }

          Logger::trace("server: got connection from {}", s);
          Logger::trace("Client connected");

          {
            //std::lock_guard<std::mutex> guard(_mutexSockets);
            _socket = socket;
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
        //std::lock_guard<std::mutex> guard(_mutexSockets);
        int status = ::send(_socket, message.data(), static_cast<int>(message.size()), 0);
        if (status == -1) {
          int err = WSAGetLastError();
          Logger::error("Can't send data: [{}] {}", err, gai_strerrorA(err));
        }
        Logger::trace("sended: {}", message.data());
      }
    }

    void WinServer::read()
    {
      std::array<pollfd, 1> sockets;
      sockets[0].fd = _socket;
      sockets[0].events = POLLIN;
      const int timeout{ 1000 };//1s
      bool hangup{ false };
      int status{ 0 };
      std::string message;

      while (!hangup) {
        status = WSAPoll(sockets.data(), sockets.size(), timeout);
        if (status == 0) {
          Logger::trace("poll timeout");
        } else if (status == SOCKET_ERROR) {
          int err = WSAGetLastError();
          Logger::warn("Error will polling: [{}] {}", err, gai_strerrorA(err));
          hangup = true;
        } else {
          int events = sockets[0].revents & POLLIN;
          if (events) {
            Logger::trace("events poll for socket id: {}", sockets[0].fd);
            const int size = 10000;
            std::vector<char> buffer(size);//tmp
            int recvstatus{ 0 };

            message.clear();

            do {
              recvstatus = ::recv(_socket, buffer.data(), size, 0);
              message.append(buffer.data());
              if (std::strcmp(buffer.data(), "\0") == 0) {
                recvstatus = -1;
              }
              Logger::warn("status: {} msg: {}", recvstatus, message);
            } while (recvstatus > 0);
              if (message == "quit") hangup = true;

              //@todo should send a notification catch by the manager, api_manager should not be here
              //_api_manager->received(message);
          } else {
            Logger::warn("unexpected events poll for socket id: {}", sockets[0].fd);
            perror("send");
            hangup = true;
          }
        }
      }
    }
  #else
    //@todo linux impl
  #endif

  void Server::bind(std::string const& port)
  {
    _pimpl->bind(port);
  }

  void Server::close()
  {
    _pimpl->close();
  }

  void Server::listen()
  {
    _pimpl->listen();
  }

  void Server::read()
  {
    _pimpl->read();
  }

  void Server::send(std::string message)
  {
    _pimpl->send(message);
  }
}
