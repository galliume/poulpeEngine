#pragma once

namespace Poulpe
{
  enum class SocketStatus {
    NOT_CONNECTED,
    CONNECTED
  };

  class ISocket
  {
  public:
    virtual ~ISocket() = default;

    virtual void close() = 0;
    virtual void bind(std::string const& ip, unsigned short const port) = 0;
    virtual void connect() = 0;
    virtual inline std::string getIP() = 0;
    virtual inline unsigned short getPort() = 0;
    virtual void listen() = 0;
    virtual void read() = 0;
  };
}
