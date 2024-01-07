#pragma once

#include "ILightManager.hpp"

namespace Poulpe
{
    class LightManager : public ILightManager
    {
    public:

        LightManager();
        ~LightManager() override = default;

        void animateAmbientLight(float deltaTime) override;
        inline Light getAmbientLight() override { return m_AmbientLight; }
        inline std::vector<Light> getPointLights() override { return m_PointLights; }
        inline std::vector<Light> getSpotLights() override { return m_SpotLights; }

    private:
        Light m_AmbientLight{};
        std::vector<Light> m_PointLights{};
        std::vector<Light> m_SpotLights{};
    };
}
