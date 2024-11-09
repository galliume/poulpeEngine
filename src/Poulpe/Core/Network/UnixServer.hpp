#pragma once

#if __linux__

//@todo to impl
namespace Poulpe
{
  class UnixServer
  {
  public:
    void bind(std::string const& port) {};
    void close() {};
    inline ServerStatus getStatus() {};
    void listen() {};
    void read() {};
    void send(std::string message) {};
  };
}

#endif
