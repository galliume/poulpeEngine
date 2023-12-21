#pragma once

#include "IRenderer.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class AbstractRenderer : public IVisitor, public IRenderer
    {
    public:
        AbstractRenderer();
        ~AbstractRenderer();

    void init(VulkanAdapter* adapter, TextureManager* textureManager, LightManager* lightManager) override;

    protected:
        VulkanAdapter* m_Adapter;
        TextureManager* m_TextureManager;
        LightManager* m_LightManager;
    };
}