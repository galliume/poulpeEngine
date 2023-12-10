#pragma once

#include "ILightManager.hpp"

#include "Poulpe/Core/Buffer.hpp"

namespace Poulpe
{
    class LightManager : ILightManager
    {
    public:

        LightManager();
        ~LightManager() = default;

        Light getAmbientLight() { return m_AmbientLight; }

    private:
        Light m_AmbientLight;
        std::vector<Light> m_DirLights{};
    };
}
