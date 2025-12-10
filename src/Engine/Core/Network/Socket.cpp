module;
#if defined(_WIN64)
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  //@todo linux impl
#endif

module Engine.Core.Network.Socket;

import std;

namespace Poulpe
{
  #if defined(_WIN64)
    WinSocket::WinSocket()
    {
      WSAStartup(MAKEWORD(2, 2), &_data);

      _socket = ::socket(AF_INET6, SOCK_STREAM, 0);

      if (INVALID_SOCKET == _socket) {
        throw std::runtime_error("Error while creating the socket");
      }
    }

    WinSocket::~WinSocket()
    {
      ::closesocket(_socket);
      ::WSACleanup();
    }

    void WinSocket::close()
    {
      if (_status == SocketStatus::NOT_CONNECTED) {
        Logger::trace("socket already disconnected");
        return;
      }

      closesocket(_socket);

      int status = WSAGetLastError();

      if (0 != status) {
        Logger::error("Close socket failed {}", status);
      }

      _status = SocketStatus::NOT_CONNECTED;
    }

    void WinSocket::bind(std::string const& ip, unsigned short const port)
    {
      ::inet_pton(AF_INET6, ip.c_str(), & _sockAddrIn.sin6_addr);
      _sockAddrIn.sin6_family = AF_INET;
      _sockAddrIn.sin6_port = ::htons(port);

      ::bind(_socket, (SOCKADDR*)&_sockAddrIn, sizeof(_sockAddrIn));

      int status = WSAGetLastError();

      if (0 != status) {
        Logger::error("bind socket failed {}", status);
      }

      _ip = ip;
      _port = port;
    }

    void WinSocket::connect()
    {
      ::connect(_socket, (SOCKADDR*) & _sockAddrIn, sizeof(_sockAddrIn));

      int status = WSAGetLastError();

      if (0 != status) {
        Logger::error("connect socket failed {}", status);
      }

      Logger::trace("ClientSocket Connected");
    }

    void WinSocket::listen()
    {
      ::listen(_socket, 0); //@todo fix max connection?
    }

    void WinSocket::read()
    {
      char recvbuf[512];
      int recvbuflen = 512;

      int status{ 0 };

      do {
        status = ::recv(_socket, recvbuf, recvbuflen, 0);
        Logger::trace("bytes received {}", status);
        Logger::trace("received {}", recvbuf);
      } while (status > 0);

      closesocket(_socket);
      WSACleanup();
    }
  #else
    //todo linux impl
  #endif

  #if defined(_WIN64)
  Socket::Socket(WinSocket* socket)
  {
    _pimpl = std::unique_ptr<WinSocket>(socket);
  }
  #else
  Socket::Socket(UnixSocket* socket)
  {
    _pimpl = std::unique_ptr<UnixSocket>(socket);
  }
  #endif

  Socket::~Socket()
  {
    _pimpl->close();
  }

  void Socket::close()
  {
    _pimpl->close();
  }

  void Socket::bind(std::string const& ip, unsigned short const port)
  {
    _pimpl->bind(ip, port);
  }

  void Socket::connect()
  {
    _pimpl->connect();
  }

  void Socket::listen()
  {
    _pimpl->listen();
  }

  void Socket::read()
  {
    _pimpl->read();
  }
}
