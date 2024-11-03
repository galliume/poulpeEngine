#pragma once

#if defined(_WIN32) || defined(WIN32)

#include "Poulpe/Core/PlpTypedef.hpp"

#include <WinSock2.h>
#include <WS2tcpip.h>

namespace Poulpe
{
  class WinSocket
  {
  public:
    WinSocket();
    ~WinSocket();

    void close();
    void bind(std::string const& ip, unsigned short const port);
    void connect();
    inline std::string getIP() { return m_IP; }
    inline unsigned short getPort() { return m_Port; }
    void listen();
    void read();

  private:
    WSADATA m_Data;
    SOCKET m_Socket;
    SOCKADDR_IN6 m_SockAddrIn;
    SocketStatus m_Status{ SocketStatus::NOT_CONNECTED};

    std::string m_IP;
    unsigned short m_Port;
  };
}

#endif
