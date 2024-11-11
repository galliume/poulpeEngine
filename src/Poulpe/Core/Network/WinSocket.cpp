#include "WinSocket.hpp"

#if defined(_WIN32) || defined(WIN32)

//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
#include <WinSock2.h>
#include <WS2tcpip.h>

namespace Poulpe
{
  WinSocket::WinSocket()
  {
    WSAStartup(MAKEWORD(2, 2), &_data);

    _Socket = ::socket(AF_INET6, SOCK_STREAM, 0);

    if (INVALID_SOCKET == _Socket) {
      throw std::runtime_error("Error while creating the socket");
    }
  }

  WinSocket::~WinSocket()
  {
   ::closesocket(_Socket);
   ::WSACleanup();
  }

  void WinSocket::close()
  {
    if (_Status == SocketStatus::NOT_CONNECTED) {
      PLP_TRACE("socket already disconnected");
      return;
    }

    closesocket(_Socket);

    int status = WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("Close socket failed {}", status);
    }

    _Status = SocketStatus::NOT_CONNECTED;
  }

  void WinSocket::bind(std::string const& ip, unsigned short const port)
  {
    ::inet_pton(AF_INET6, ip.c_str(), & _SockAddrIn.sin6_addr);
    _SockAddrIn.sin6_family = AF_INET;
    _SockAddrIn.sin6_port = ::htons(port);

    ::bind(_Socket, (SOCKADDR*)&_SockAddrIn, sizeof(_SockAddrIn));

    int status = WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("bind socket failed {}", status);
    }

    _IP = ip;
    _Port = port;
  }

  void WinSocket::connect()
  {
    ::connect(_Socket, (SOCKADDR*) & _SockAddrIn, sizeof(_SockAddrIn));

    int status = WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("connect socket failed {}", status);
    }

    PLP_TRACE("ClientSocket Connected");
  }

  void WinSocket::listen()
  {
    ::listen(_Socket, 0); //@todo fix max connection?
  }

  void WinSocket::read()
  {
    char recvbuf[512];
    int recvbuflen = 512;

    int status{ 0 };

    do {
      status = ::recv(_Socket, recvbuf, recvbuflen, 0);
      PLP_TRACE("bytes received {}", status);
      PLP_TRACE("received {}", recvbuf);
    } while (status > 0);

    closesocket(_Socket);
    WSACleanup();
  }
}

#endif
