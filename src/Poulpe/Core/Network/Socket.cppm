module;
#include <string>
#include <memory>

export module Poulpe.Core.Network.Socket;

import Poulpe.Core.Network.WinSocket;

namespace Poulpe
{
  export class Socket
  {
  public:
    Socket(WinSocket* socket);
    ~Socket();

    void close();
    void bind(std::string const& ip, unsigned short const port);
    void connect();
    inline std::string getIP() { return _pimpl->getIP(); }
    inline unsigned short getPort() { return _pimpl->getPort(); }
    void listen();
    void read();

  private:
    std::unique_ptr<WinSocket> _pimpl;
  };
}
