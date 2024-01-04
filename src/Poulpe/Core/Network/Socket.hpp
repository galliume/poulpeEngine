#pragma once

#include "ISocket.hpp"

namespace Poulpe
{
  class Socket
  {
    Socket() = default;
    ~Socket() = default;

  private:
    std::unique_ptr<Socket> m_pimpl;
  };
}