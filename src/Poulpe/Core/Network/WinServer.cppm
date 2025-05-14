export module Poulpe.Core.Network:WinServer;

#if defined(_WIN32) || defined(WIN32)

import Poulpe.Core.PlpTypedef;
import Poulpe.Manager.APIManager;

#include <WinSock2.h>

import <mutex>

export class WinServer
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
  APIManager* _api_manager;
};

#endif
