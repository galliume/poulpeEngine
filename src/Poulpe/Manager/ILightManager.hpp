#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

namespace Poulpe
{
  class ILightManager
  {
  public:
    ILightManager() = default;
    virtual ~ILightManager() = default;
    
    virtual void animateAmbientLight(float deltaTime) = 0;
    virtual inline Light getAmbientLight() = 0;
    virtual inline std::vector<Light> getPointLights() = 0;
    virtual inline std::vector<Light> getSpotLights() = 0;
  };
}
