module;

#include <chrono>

export module Engine.Managers.LightManager;

import Engine.Core.PlpTypedef;

namespace Poulpe
{
  export class LightManager
  {
  public:

    LightManager();

    void animateAmbientLight(double const delta_time);
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
