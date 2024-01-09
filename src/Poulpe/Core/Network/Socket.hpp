#pragma once

#include "ISocket.hpp"

namespace Poulpe
{
  class Socket : public ISocket
  {
  public:
    Socket(ISocket* socket);
    ~Socket() override;

    void close() override;
    void bind(std::string const& ip, unsigned short const port) override;
    void connect() override;
    inline std::string getIP() override { return m_Pimpl->getIP(); }
    inline unsigned short getPort() override { return m_Pimpl->getPort(); }
    void listen() override;
    void read() override;

  private:
    std::unique_ptr<ISocket> m_Pimpl;
  };
}
