#pragma once

#include "ISocket.hpp"

namespace Poulpe
{
  class WinSocket : public ISocket
  {
    WinSocket() = default;
    ~WinSocket() = default;
  };
}