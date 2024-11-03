#pragma once

namespace Poulpe
{
  class Socket
  {
  public:
    Socket(Socket* socket);
    ~Socket();

    void close();
    void bind(std::string const& ip, unsigned short const port);
    void connect();
    inline std::string getIP() { return m_Pimpl->getIP(); }
    inline unsigned short getPort() { return m_Pimpl->getPort(); }
    void listen();
    void read();

  private:
    std::unique_ptr<Socket> m_Pimpl;
  };
}
