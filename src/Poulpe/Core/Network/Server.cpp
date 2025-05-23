module;
#include <string>

module Poulpe.Core.Network.Server;

namespace Poulpe
{
  void Server::bind(std::string const& port)
  {
    _pimpl->bind(port);
  }

  void Server::close()
  {
    _pimpl->close();
  }

  void Server::listen()
  {
    _pimpl->listen();
  }

  void Server::read()
  {
    _pimpl->read();
  }

  void Server::send(std::string message)
  {
    _pimpl->send(message);
  }
}
