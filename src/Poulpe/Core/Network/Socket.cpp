module;
#include <memory>

module Poulpe.Core.Network.Socket;

namespace Poulpe
{
  Socket::Socket(WinSocket* socket)
  {
    _pimpl = std::unique_ptr<WinSocket>(socket);
  }

  Socket::~Socket()
  {
    _pimpl->close();
  }

  void Socket::close()
  {
    _pimpl->close();
  }

  void Socket::bind(std::string const& ip, unsigned short const port)
  {
    _pimpl->bind(ip, port);
  }

  void Socket::connect()
  {
    _pimpl->connect();
  }

  void Socket::listen()
  {
    _pimpl->listen();
  }

  void Socket::read()
  {
    _pimpl->read();
  }
}
