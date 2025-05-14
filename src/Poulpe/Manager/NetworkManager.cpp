module Poulpe.Manager.NetworkManager;

namespace Poulpe
{
  NetworkManager::NetworkManager(APIManager* apiManager)
    : _api_manager(apiManager)
  {

  }

  void NetworkManager::startServer(std::string const& port)
  {
    _Server = std::make_shared<Server>(new WinServer(_api_manager));

    std::jthread listen([this, &port]() {
      _Server->bind(port);
      _Server->listen();
    });
    listen.detach();
  }

  void NetworkManager::received(std::string const& message)
  {
    Logger::trace("received {}", message);
  }
}
