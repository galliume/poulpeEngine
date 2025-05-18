module;

#include <WinSock2.h>

#include <stdexcept>
#include <mutex>

export module Poulpe.Core.Network.WinServer;

import Poulpe.Core.Logger;
import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  export class WinServer
  {
  public:
    WinServer() = default;
    ~WinServer();

    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return _status; }
    void listen();
    void read();
    void send(std::string message);

  private:
    SOCKET _servSocket;
    WSADATA _data;
    //sockaddr_in6 _SocketAddr;

    SOCKET _socket{ 0 };

    std::mutex _mutexSockets;

    ServerStatus _status{ ServerStatus::NOT_RUNNING };
  };
}
