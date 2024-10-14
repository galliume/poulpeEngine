#pragma once

#include <nlohmann/json.hpp>

namespace Poulpe
{
  class IInputManager
  {
  public:
    IInputManager() = default;
    ~IInputManager() = default;

    virtual void init(nlohmann::json const& inputConfig) = 0;
  };
}
