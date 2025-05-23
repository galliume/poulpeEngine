module;

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <minwindef.h>

#include <string>

export module Poulpe.Core.Network.WinSocket;

import Poulpe.Core.Logger;
import Poulpe.Core.Network.Server;
import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  export class WinSocket
  {
  public:
    WinSocket();
    ~WinSocket();

    void close();
    void bind(std::string const& ip, unsigned short const port);
    void connect();
    inline std::string getIP() { return _ip; }
    inline unsigned short getPort() { return _port; }
    void listen();
    void read();

  private:
    WSADATA _data;
    SOCKET _socket;
    SOCKADDR_IN6 _sockAddrIn;
    SocketStatus _status{ SocketStatus::NOT_CONNECTED};

    std::string _ip;
    unsigned short _port;
  };
}
