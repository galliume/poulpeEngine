#pragma once

#include "IServer.hpp"

namespace Poulpe
{
  class Server
  {
  public:
    Server() = default;
    ~Server() = default;

  private:
    std::unique_ptr<IServer> m_pimpl;
  };
}
