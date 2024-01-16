#pragma once

#include <nlohmann/json.hpp>

namespace Poulpe
{
  class IAPIManager
  {
  public:
    IAPIManager() = default;
    virtual ~IAPIManager() = default;

    virtual void received(std::string const& message) = 0;
  };
}
