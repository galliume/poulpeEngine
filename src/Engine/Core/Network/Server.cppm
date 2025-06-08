module;

#if defined(_WIN64)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <ws2ipdef.h>

  #else
  //@todo linux impl
  #endif
  
#include <cstring>
#include <mutex>
#include <string>
#include <stdexcept>

export module Engine.Core.Network.Server;

import Engine.Core.Logger;
import Engine.Core.PlpTypedef;

namespace Poulpe
{

  #if defined(_WIN64)
    export class WinServer
    {
    public:
      WinServer() = default;
      ~WinServer();

      void bind(std::string const& port);
      void close();
      inline ServerStatus getStatus() { return _status; }
      void listen();
      void read();
      void send(std::string message);

    private:
      SOCKET _servSocket;
      WSADATA _data;
      //sockaddr_in6 _SocketAddr;

      SOCKET _socket{ 0 };

      std::mutex _mutexSockets;

      ServerStatus _status{ ServerStatus::NOT_RUNNING };
    };
  #else
  //@todo linux impl
    export class UnixServer
    {
    public:
      void bind(std::string const& port) {};
      void close() {};
      inline ServerStatus getStatus() { return _status; };
      void listen() {};
      void read() {};
      void send(std::string message) {};

      ServerStatus _status{ ServerStatus::NOT_RUNNING };

    };
  #endif

  export class Server
  {
  public:
    #if defined(_WIN64)
      Server(WinServer* server)
        : _pimpl(server)
      {
      }
    #else
      Server(UnixServer* server)
        : _pimpl(server)
      {
      }
    #endif


    void bind(std::string const& port);
    void close();
    inline ServerStatus getStatus() { return _pimpl->getStatus(); }
    void listen();
    void read();
    void send(std::string message);

  private:
    
    #if defined(_WIN64)
      WinServer* _pimpl;
    #else
      UnixServer* _pimpl;
    #endif
  };
}
