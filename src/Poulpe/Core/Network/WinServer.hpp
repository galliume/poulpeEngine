#pragma once

#if defined(_WIN32) || defined(WIN32)

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Manager/APIManager.hpp"

#include <WinSock2.h>

#include <mutex>

namespace Poulpe
{
  class WinServer
  {
  public:
    WinServer(APIManager* APIManager);
    ~WinServer();

    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return m_Status; }
    void listen();
    void read();
    void send(std::string message);

  private:
    SOCKET m_ServSocket;
    WSADATA m_Data;
    //sockaddr_in6 m_SocketAddr;

    SOCKET m_Socket{ 0 };

    std::mutex m_MutexSockets;

    ServerStatus m_Status{ ServerStatus::NOT_RUNNING };
    APIManager* m_APIManager;
  };
}

#endif
