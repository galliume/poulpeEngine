module;
#include <memory>
#include <string>
#include <thread>

module Poulpe.Managers;

import Poulpe.Core.Logger;
import Poulpe.Core.Network.Server;
import Poulpe.Core.Network.WinServer;

namespace Poulpe
{
  NetworkManager::NetworkManager(APIManager* apiManager)
    : _api_manager(apiManager)
  {

  }

  void NetworkManager::startServer(std::string const& port)
  {
    //_server = std::make_shared<Server>(new WinServer(_api_manager));
    _server = std::make_shared<Server>(new WinServer());

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
