#include "AbstractEntity.hpp"

namespace Poulpe
{
    void AbstractRenderer::init(VulkanAdapter* adapter, TextureManager* textureManager, LightManager* lightManager)
    {
        m_Adapter = adapter;
        m_TextureManager = textureManager;
        m_LightManager = lightManager;
    }
}