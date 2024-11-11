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
    inline std::string getIP() { return _IP; }
    inline unsigned short getPort() { return _Port; }
    void listen();
    void read();

  private:
    WSADATA _data;
    SOCKET _Socket;
    SOCKADDR_IN6 _SockAddrIn;
    SocketStatus _Status{ SocketStatus::NOT_CONNECTED};

    std::string _IP;
    unsigned short _Port;
  };
}

#endif
