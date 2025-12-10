export module Engine.Managers.NetworkManager;

import std;

import Engine.Core.Network.Server;

import Engine.Managers.APIManager;

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
