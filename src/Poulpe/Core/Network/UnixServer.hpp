#pragma once

#include "IServer.hpp"

namespace Poulpe
{
  class UnixServer : public IServer
  {
    UnixServer() = default;
    ~UnixServer() = default;
  };
}