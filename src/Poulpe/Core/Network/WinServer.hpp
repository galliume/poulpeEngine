#pragma once

#include "IServer.hpp"

#if defined(_WIN32) || defined(WIN32)

#include <WinSock2.h>

namespace Poulpe
{
  class WinServer : public IServer
  {
  public:
    WinServer() = default;
    ~WinServer() override;

    void bind(std::string const& port) override;
    void close() override;
    inline ServerStatus getStatus() override { return m_Status; }
    void listen() override;
    void read() override;
    void send(std::string message) override;

  private:
    SOCKET m_ServSocket;
    WSADATA m_Data;
    //sockaddr_in6 m_SocketAddr;

    SOCKET m_Socket;

    std::mutex m_MutexSockets;

    ServerStatus m_Status{ ServerStatus::NOT_RUNNING };
  };
}

#endif
