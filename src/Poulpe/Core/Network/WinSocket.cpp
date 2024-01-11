#include "WinSocket.hpp"

#if defined(_WIN32) || defined(WIN32)

//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
#include <WinSock2.h>
#include <WS2tcpip.h>

namespace Poulpe
{
  WinSocket::WinSocket()
  {
    WSAStartup(MAKEWORD(2, 2), &m_Data);

    m_Socket = ::socket(AF_INET6, SOCK_STREAM, 0);

    if (INVALID_SOCKET == m_Socket) {
      throw std::runtime_error("Error while creating the socket");
    }
  }

  WinSocket::~WinSocket()
  {
   ::closesocket(m_Socket);
   ::WSACleanup();
  }

  void WinSocket::close()
  {
    if (m_Status == SocketStatus::NOT_CONNECTED) {
      PLP_TRACE("socket already disconnected");
      return;
    }

    closesocket(m_Socket);

    int status = WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("Close socket failed {}", status);
    }

    m_Status = SocketStatus::NOT_CONNECTED;
  }

  void WinSocket::bind(std::string const& ip, unsigned short const port)
  {
    ::inet_pton(AF_INET6, ip.c_str(), & m_SockAddrIn.sin6_addr);
    m_SockAddrIn.sin6_family = AF_INET;
    m_SockAddrIn.sin6_port = ::htons(port);

    ::bind(m_Socket, (SOCKADDR*)&m_SockAddrIn, sizeof(m_SockAddrIn));

    int status = WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("bind socket failed {}", status);
    }

    m_IP = ip;
    m_Port = port;
  }

  void WinSocket::connect()
  {
    ::connect(m_Socket, (SOCKADDR*) & m_SockAddrIn, sizeof(m_SockAddrIn));

    int status = WSAGetLastError();

    if (0 != status) {
      PLP_ERROR("connect socket failed {}", status);
    }

    PLP_TRACE("ClientSocket Connected");
  }

  void WinSocket::listen()
  {
    ::listen(m_Socket, 0); //@todo fix max connection?
  }

  void WinSocket::read()
  {
    char recvbuf[512];
    int recvbuflen = 512;

    int status{ 0 };

    do {
      status = ::recv(m_Socket, recvbuf, recvbuflen, 0);
      PLP_TRACE("bytes received {}", status);
      PLP_TRACE("received {}", recvbuf);
    } while (status > 0);

    closesocket(m_Socket);
    WSACleanup();
  }
}

#endif
