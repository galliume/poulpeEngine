#pragma once

namespace Poulpe
{
  class Server;
  class APIManager;

  class NetworkManager
  {
  public:
    NetworkManager(APIManager* apiManager);
    
    void received(std::string const& notification);
    void startServer(std::string const& port);

  private:
    std::shared_ptr<Server> m_Server;
    APIManager* m_APIManager;
  };
}
