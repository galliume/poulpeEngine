#pragma once

#include "IComponentRenderer.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class AbstractRenderer : public IVisitor, public IComponentRenderer
    {
    public:
        AbstractRenderer() = default;
        ~AbstractRenderer() = default;

    void init(IRenderer* const renderer,
        ITextureManager* const textureManager,
        ILightManager* const lightManager) override;

    protected:
        IRenderer* m_Renderer;
        ITextureManager* m_TextureManager;
        ILightManager* m_LightManager;
    };
}