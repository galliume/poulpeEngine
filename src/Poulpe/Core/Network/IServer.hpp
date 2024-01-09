#pragma once

namespace Poulpe
{
    enum class ServerStatus {
      NOT_RUNNING,
      RUNNING
    };

  class IServer
  {
  public:
    virtual ~IServer() = default;

    virtual void bind(std::string const& port) = 0;
    virtual void close() = 0;
    virtual inline ServerStatus getStatus() = 0;
    virtual void listen() = 0;
    virtual void read() = 0;
    virtual void send(std::string message) = 0;
  };
}
