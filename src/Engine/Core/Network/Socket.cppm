module;

#if defined(_WIN64)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <minwindef.h>
#else
  //@todo linux impl
#endif

#include <string>
#include <memory>

export module Engine.Core.Network.Socket;

import Engine.Core.Logger;
import Engine.Core.PlpTypedef;

namespace Poulpe
{
  #if defined(_WIN64)
  export class WinSocket
    {
    public:
      WinSocket();
      ~WinSocket();

      void close();
      void bind(std::string const& ip, unsigned short const port);
      void connect();
      inline std::string getIP() { return _ip; }
      inline unsigned short getPort() { return _port; }
      void listen();
      void read();

    private:
      WSADATA _data;
      SOCKET _socket;
      SOCKADDR_IN6 _sockAddrIn;
      SocketStatus _status{ SocketStatus::NOT_CONNECTED};

      std::string _ip;
      unsigned short _port;
    };
  #else
    //@todo linux impl
    export class UnixSocket
    {
    public:
      UnixSocket() = default;
      ~UnixSocket() = default;

      void close() {}
      void bind(std::string const& ip, unsigned short const port) {}
      void connect() {}
      inline std::string getIP() { return _ip; }
      inline unsigned short getPort() { return _port; }
      void listen() {}
      void read() {}

      private:
        //SocketStatus _status{ SocketStatus::NOT_CONNECTED};

        std::string _ip;
        unsigned short _port;
    };
  #endif

  export class Socket
  {
  public:
    #if defined(_WIN64)
      Socket(WinSocket* socket);
    #else
      Socket(UnixSocket* socket);
    #endif
    ~Socket();

    void close();
    void bind(std::string const& ip, unsigned short const port);
    void connect();
    inline std::string getIP() { return _pimpl->getIP(); }
    inline unsigned short getPort() { return _pimpl->getPort(); }
    void listen();
    void read();

  private:
    #if defined(_WIN64)
      std::unique_ptr<WinSocket> _pimpl;
    #else
      std::unique_ptr<UnixSocket> _pimpl;
    #endif
  };
}
