#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

namespace Poulpe
{
  class LightManager
  {
  public:

    LightManager();

    void animateAmbientLight(float deltaTime);
    inline Light getAmbientLight() { return _AmbientLight; }
    inline std::vector<Light> getPointLights() { return _PointLights; }
    inline std::vector<Light> getSpotLights() { return _SpotLights; }

  private:
    Light _AmbientLight{};
    std::vector<Light> _PointLights{};
    std::vector<Light> _SpotLights{};
  };
}
