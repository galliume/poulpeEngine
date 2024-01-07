#pragma once

#include "ISocket.hpp"

namespace Poulpe
{
  class UnixSocket : public ISocket
  {
  public:
    UnixSocket() = default;
    ~UnixSocket() = default;
  };
}