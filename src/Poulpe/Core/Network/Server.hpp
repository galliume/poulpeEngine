#pragma once

#include "IServer.hpp"

namespace Poulpe
{
  class Server : public IServer
  {
  public:
    Server(IServer* server);
    ~Server() override = default;

    void bind(std::string const& port) override;
    void close() override;
    inline ServerStatus getStatus() override { return m_Pimpl->getStatus(); }
    void listen() override;
    void read() override;
    void send(std::string message) override;

  private:
    IServer* m_Pimpl;
  };
}
