module;
#include <string>
#include <memory>

export module Poulpe.Core.Network.Socket;

namespace Poulpe
{
  export class Socket
  {
  public:
    Socket(Socket* socket);
    ~Socket();

    void close();
    void bind(std::string const& ip, unsigned short const port);
    void connect();
    inline std::string getIP() { return _Pimpl->getIP(); }
    inline unsigned short getPort() { return _Pimpl->getPort(); }
    void listen();
    void read();

  private:
    std::unique_ptr<Socket> _Pimpl;
  };
}
