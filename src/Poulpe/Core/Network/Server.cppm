module;
#include <string>

export module Poulpe.Core.Network.Server;

import Poulpe.Core.Network.WinServer;

import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  export class Server
{
public:
  Server(WinServer* server)
    : _Pimpl(server)
  {
  }
  // Server(UnixServer* server)
  //   : _Pimpl(server)
  // {
  // }


  void bind(std::string const& port);
  void close();
  inline ServerStatus getStatus() { return _Pimpl->getStatus(); }
  void listen();
  void read();
  void send(std::string message);

private:
  
  WinServer* _Pimpl;
//   UnixServer* _Pimpl;
};
}
