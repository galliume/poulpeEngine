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
  NetworkManager::NetworkManager(IAPIManager* apiManager)
    : m_APIManager(apiManager)
  {

  }

  void NetworkManager::startServer(std::string const& port)
  {
    #if defined(_WIN32) || defined(WIN32)
    m_Server = std::make_shared<Server>(new WinServer(m_APIManager));

    Locator::getThreadPool()->submit("winServerListen", [this, &port]() {
      m_Server->bind(port);
      m_Server->listen();
    });

    #endif
  }

  void NetworkManager::received(std::string const& message)
  {
    PLP_TRACE("received {}", message);
  }
}
