#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

//@todo fix Win/Linux impl
#include "WinServer.hpp"

namespace Poulpe
{
  class Server
  {
  public:
    Server(WinServer* server)
      : m_Pimpl(server)
    {
    }

    ~Server() = default;

    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return m_Pimpl->getStatus(); }
    void listen();
    void read();
    void send(std::string message);

  private:
    //@todo fixed pimpl...
    WinServer* m_Pimpl;
  };
}
