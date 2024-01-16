#pragma once

#include "INetworkManager.hpp"
#include "IAPIManager.hpp"

namespace Poulpe
{
  class Server;

  class NetworkManager : public INetworkManager
  {
  public:
    NetworkManager(IAPIManager* apiManager);
    ~NetworkManager() override = default;

    void received(std::string const& notification) override;
    void startServer(std::string const& port) override;

  private:
    std::shared_ptr<Server> m_Server;
    [[maybe_unused]]IAPIManager* m_APIManager;
  };
}
