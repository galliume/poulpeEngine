#include "NetworkManager.hpp"

#include "Poulpe/Core/Network/Server.hpp"
#include "Poulpe/Core/Network/Socket.hpp"

//@todo detect Unix
#if defined(_WIN32) || defined(WIN32)
#include "Poulpe/Core/Network/WinServer.hpp"
#include "Poulpe/Core/Network/WinSocket.hpp"
#endif

namespace Poulpe
{
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
}
