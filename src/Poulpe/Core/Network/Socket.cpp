#include "Socket.hpp"

namespace Poulpe
{
  Socket::Socket(ISocket* socket)
  {
    m_Pimpl = std::unique_ptr<ISocket>(socket);
  }
}
