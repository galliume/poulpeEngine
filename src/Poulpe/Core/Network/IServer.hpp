#pragma once

namespace Poulpe
{
  class IServer
  {
  public:
    virtual ~IServer() = default;

    virtual void bind(std::string const& port) = 0;
    virtual void listen() = 0;
    virtual void read() = 0;
    virtual void send(std::string message) = 0;
  };
}
