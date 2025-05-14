module;

#include <memory>
#include <string>

export module Poulpe.Manager.NetworkManager;

namespace Poulpe
{
  //@todo detect Unix
  // #if defined(_WIN32) || defined(WIN32)

  // #endif

  import Poulpe.Manager.APIManager;
  import Poulpe.Core.Network.Server;

  export class NetworkManager
  {
  public:
    NetworkManager(APIManager* apiManager);

    void received(std::string const& notification);
    void startServer(std::string const& port);

  private:
    std::shared_ptr<Server> _Server;
    APIManager* _api_manager;
  };
}
