#include "AbstractRenderer.hpp"

namespace Poulpe
{
    void AbstractRenderer::init(
        IRenderer* const renderer,
        ITextureManager* const textureManager,
        ILightManager* const lightManager)
    {
        m_Renderer = renderer;
        m_TextureManager = textureManager;
        m_LightManager = lightManager;
    }
}
