#pragma once

#include "ISocket.hpp"

#include <WinSock2.h>

namespace Poulpe
{
  class WinSocket : public ISocket
  {
  public:
    WinSocket();
    ~WinSocket();

    int checkError();
  private:
    WSADATA m_Data;
    SOCKET m_Socket;
  };
}
