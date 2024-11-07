#include "Server.hpp"

namespace Poulpe
{
  void Server::bind(std::string const& port)
  {
    m_Pimpl->bind(port);
  }

  void Server::close()
  {
    m_Pimpl->close();
  }

  void Server::listen()
  {
    m_Pimpl->listen();
  }

  void Server::read()
  {
    m_Pimpl->read();
  }

  void Server::send(std::string message)
  {
    m_Pimpl->send(message);
  }
}
