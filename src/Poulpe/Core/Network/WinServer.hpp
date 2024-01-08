#pragma once

#include "IServer.hpp"

#include <WinSock2.h>

namespace Poulpe
{
  class WinServer : public IServer
  {
  public:
    WinServer() = default;
    ~WinServer() override;

    void bind(std::string const& port) override;
    void listen() override;
    void read() override;
    void send(std::string message) override;

  private:
    SOCKET m_ServSocket;
    WSADATA m_Data;

    std::vector<SOCKET> m_Sockets;

    std::mutex m_MutexSockets;
  };
}
