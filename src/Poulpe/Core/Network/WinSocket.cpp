#include "WinSocket.hpp"

//https://learn.microsoft.com/en-us/windows/win32/api/winsock2/
#include <WinSock2.h>

namespace Poulpe
{
  WinSocket::WinSocket()
  {
    WSAStartup(MAKEWORD(2, 2), &m_Data);
    checkError();

    m_Socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (INVALID_SOCKET == m_Socket) {
      checkError();
      throw std::runtime_error("Error while creating the socket");
    }
  }

  WinSocket::~WinSocket()
  {
    if (0 != closesocket(m_Socket)) {
      checkError();
    }

    if (0 != WSACleanup()) {
      checkError();
    }
  }

  //https://learn.microsoft.com/fr-fr/windows/win32/winsock/windows-sockets-error-codes-2?redirectedfrom=MSDN
  int WinSocket::checkError()
  {
    int error = WSAGetLastError();

    if (0 != error) {
      switch (error) {
      case WSA_INVALID_HANDLE:
        PLP_ERROR("Invalid socket handle");
        break;
      case WSA_NOT_ENOUGH_MEMORY:
        PLP_ERROR("Not enough memory");
        break;
      case WSA_INVALID_PARAMETER:
        PLP_ERROR("Invalid parameter");
        break;
      case WSA_OPERATION_ABORTED:
        PLP_ERROR("Operation aborted");
        break;
      case WSA_IO_INCOMPLETE:
        PLP_ERROR("Operation incomplete");
        break;
      case WSA_IO_PENDING:
        PLP_ERROR("Operation pending");
        break;
      case WSAEINTR:
        PLP_ERROR("Operation canceled");
        break;
      case WSAEBADF:
        PLP_ERROR("File handle invalid");
        break;
      case WSAEACCES:
        PLP_ERROR("Unauthorized socket access");
        break;
      case WSAEFAULT:
        PLP_ERROR("Unknown address");
        break;
      case WSAEINVAL:
        PLP_ERROR("Invalid argument");
        break;
      case WSAEMFILE:
        PLP_ERROR("Too much socket open");
        break;
      case WSAEWOULDBLOCK:
        PLP_ERROR("Resource temporarly unavailable");
        break;
      case WSAEINPROGRESS:
        PLP_ERROR("Operation in progress");
        break;
      case WSAEALREADY:
        PLP_ERROR("Operation already started");
        break;
      case WSAEPROTONOSUPPORT:
        PLP_ERROR("Unsuported protocol");
        break;
      case WSAESOCKTNOSUPPORT:
        PLP_ERROR("Unsuported socket type");
        break;
      case WSAEADDRINUSE:
        PLP_ERROR("Address in use");
        break;
      case WSAEADDRNOTAVAIL:
        PLP_ERROR("Address is not available");
        break;
      case WSAENETUNREACH:
        PLP_ERROR("Network unreachable");
        break;
      default:
        PLP_ERROR("Error {}", error);
        break;
      }
    }
    return error;
  }
}
