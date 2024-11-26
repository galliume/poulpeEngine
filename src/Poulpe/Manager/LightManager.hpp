#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include <chrono>

namespace Poulpe
{
  class LightManager
  {
  public:

    LightManager();

    void animateSunLight(double const delta_time);
    inline Light getSunLight() { return _sun; }
    inline std::vector<Light> getPointLights() { return _points; }
    inline std::vector<Light> getSpotLights() { return _spots; }

  private:
    Light _sun{};
    std::vector<Light> _points{};
    std::vector<Light> _spots{};
  };
}
