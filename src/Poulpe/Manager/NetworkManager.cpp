module Poulpe.Manager;

import Poulpe.Core.Network.Server;
import Poulpe.Core.Network.Socket;

//@todo detect Unix
#if defined(_WIN32) || defined(WIN32)
import Poulpe.Core.Network.WinServer;
import Poulpe.Core.Network.WinSocket;
#endif

NetworkManager::NetworkManager(APIManager* apiManager)
  : _api_manager(apiManager)
{

}

void NetworkManager::startServer(std::string const& port)
{
  #if defined(_WIN32) || defined(WIN32)
  _Server = std::make_shared<Server>(new WinServer(_api_manager));

  std::jthread listen([this, &port]() {
    _Server->bind(port);
    _Server->listen();
  });
  listen.detach();
  #endif
}

void NetworkManager::received(std::string const& message)
{
  PLP_TRACE("received {}", message);
}
