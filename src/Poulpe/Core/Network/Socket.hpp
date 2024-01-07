#pragma once

#include "ISocket.hpp"

namespace Poulpe
{
  class Socket
  {
  public:
    Socket(ISocket* socket);
    ~Socket() = default;

  private:
    std::unique_ptr<ISocket> m_Pimpl;
  };
}
