#pragma once

namespace Poulpe
{
  class Server;

  class NetworkManager
  {
  public:
    NetworkManager() = default;
    ~NetworkManager() = default;

    void startServer(std::string const& port);

  private:
    std::shared_ptr<Server> m_Server;
  };
}
