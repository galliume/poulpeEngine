module;

#include <memory>
#include <string>

export module Poulpe.Managers:NetworkManager;

import :APIManager;

import Poulpe.Core.Network.Server;

namespace Poulpe
{
  export class NetworkManager
  {
  public:
    NetworkManager(APIManager* apiManager);

    void received(std::string const& notification);
    void startServer(std::string const& port);

  private:
    std::shared_ptr<Server> _server;
    APIManager* _api_manager;
  };
}
