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
      : _Pimpl(server)
    {
    }
#else
    Server(UnixServer* server)
      : _Pimpl(server)
    {
    }
#endif

    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return _Pimpl->getStatus(); }
    void listen();
    void read();
    void send(std::string message);

  private:
    //@todo fixed pimpl...
#if defined(_WIN32) || defined(WIN32)
    WinServer* _Pimpl;
#else
    UnixServer* _Pimpl;
#endif
  };
}
