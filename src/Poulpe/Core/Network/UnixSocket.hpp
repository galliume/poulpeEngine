#pragma once

#include "ISocket.hpp"

namespace Poulpe
{
  class UnixSocket : public ISocket
  {
    UnixSocket() = default;
    ~UnixSocket() = default;
  };
}