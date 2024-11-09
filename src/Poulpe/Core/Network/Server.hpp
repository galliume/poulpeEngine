#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

//@todo fix Win/Linux impl
#include "WinServer.hpp"
#include "UnixServer.hpp"

namespace Poulpe
{
  class Server
  {
  public:
#if defined(_WIN32) || defined(WIN32)
    Server(WinServer* server)
      : m_Pimpl(server)
    {
    }
#else
    Server(UnixServer* server)
      : m_Pimpl(server)
    {
    }
#endif

    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return m_Pimpl->getStatus(); }
    void listen();
    void read();
    void send(std::string message);

  private:
    //@todo fixed pimpl...
#if defined(_WIN32) || defined(WIN32)
    WinServer* m_Pimpl;
#else
    UnixServer* m_Pimpl;
#endif
  };
}
