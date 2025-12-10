module Engine.Managers.NetworkManager;

import std;

import Engine.Core.Logger;

import Engine.Core.Network.Server;

namespace Poulpe
{
  NetworkManager::NetworkManager(APIManager* apiManager)
    : _api_manager(apiManager)
  {

  }

  void NetworkManager::startServer(std::string const& port)
  {
    //_server = std::make_shared<Server>(new WinServer(_api_manager));
    #if defined(_WIN64)
      _server = std::make_shared<Server>(new WinServer());
    #else
      _server = std::make_shared<Server>(new UnixServer());
    #endif

    std::jthread listen([this, &port]() {
      _server->bind(port);
      _server->listen();
    });
    listen.detach();
  }

  void NetworkManager::received(std::string const& message)
  {
    Logger::trace("received {}", message);
  }
}
