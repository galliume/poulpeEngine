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

        void animateAmbientLight(float deltaTime);
        inline Light getAmbientLight() { return m_AmbientLight; }
        inline std::vector<Light> getPointLights() { return m_PointLights; }
        inline std::vector<Light> getSpotLights() { return m_SpotLights; }

    private:
        Light m_AmbientLight;
        std::vector<Light> m_PointLights{};
        std::vector<Light> m_SpotLights{};
    };
}
