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
    inline ServerStatus getStatus() { return _Status; }
    void listen();
    void read();
    void send(std::string message);

  private:
    SOCKET _ServSocket;
    WSADATA _data;
    //sockaddr_in6 _SocketAddr;

    SOCKET _Socket{ 0 };

    std::mutex _MutexSockets;

    ServerStatus _Status{ ServerStatus::NOT_RUNNING };
    APIManager* _APIManager;
  };
}

#endif
