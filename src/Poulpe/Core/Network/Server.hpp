#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

namespace Poulpe
{
  class Server
  {
  public:
    Server(Server* server);
    ~Server() = default;

    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return m_Pimpl->getStatus(); }
    void listen();
    void read();
    void send(std::string message);

  private:
    //@todo fixed pimpl...
    Server* m_Pimpl;
  };
}
