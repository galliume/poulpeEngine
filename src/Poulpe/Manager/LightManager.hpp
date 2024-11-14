#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include <chrono>

namespace Poulpe
{
  class LightManager
  {
  public:

    LightManager();

    void animateAmbientLight(std::chrono::duration<float> const deltaTime);
    inline Light getAmbientLight() { return _ambient; }
    inline std::vector<Light> getPointLights() { return _points; }
    inline std::vector<Light> getSpotLights() { return _spots; }

  private:
    Light _ambient{};
    std::vector<Light> _points{};
    std::vector<Light> _spots{};
  };
}
