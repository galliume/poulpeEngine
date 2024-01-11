#pragma once

#include "ISocket.hpp"

#if defined(_WIN32) || defined(WIN32)

#include <WinSock2.h>
#include <WS2tcpip.h>

namespace Poulpe
{

  class WinSocket : public ISocket
  {
  public:
    WinSocket();
    ~WinSocket() override;

    void close() override;
    void bind(std::string const& ip, unsigned short const port) override;
    void connect() override;
    inline std::string getIP() override { return m_IP; }
    inline unsigned short getPort() override { return m_Port; }
    void listen() override;
    void read() override;

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
