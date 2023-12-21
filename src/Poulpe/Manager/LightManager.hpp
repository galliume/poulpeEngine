#pragma once

#include "ILightManager.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

namespace Poulpe
{
    class LightManager : ILightManager
    {
    public:

        LightManager();
        ~LightManager() = default;

        Light getAmbientLight() { return m_AmbientLight; }
        std::vector<Light> getPointLights() { return m_PointLights; }
        std::vector<Light> getSpotLights() { return m_SpotLights; }
        void animateAmbientLight(float deltaTime);

    private:
        Light m_AmbientLight;
        std::vector<Light> m_PointLights{};
        std::vector<Light> m_SpotLights{};
    };
}
