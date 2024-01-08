#include "Socket.hpp"

namespace Poulpe
{
  Socket::Socket(ISocket* socket)
  {
    m_Pimpl = std::unique_ptr<ISocket>(socket);
  }

  Socket::~Socket()
  {
    m_Pimpl->close();
  }

  void Socket::close()
  {
    m_Pimpl->close();
  }

  void Socket::bind(std::string const& ip, unsigned short const port)
  {
    m_Pimpl->bind(ip, port);
  }

  void Socket::connect()
  {
    m_Pimpl->connect();
  }

  void Socket::listen()
  {
    m_Pimpl->listen();
  }

  void Socket::read()
  {
    m_Pimpl->read();
  }
}
