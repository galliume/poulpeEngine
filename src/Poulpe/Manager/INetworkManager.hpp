#pragma once

namespace Poulpe
{
  class INetworkManager
  {
  public:
    INetworkManager() = default;
    virtual ~INetworkManager() = default;

    virtual void received(std::string const& notification) = 0;
    virtual void startServer(std::string const& port) = 0;
  };
}
