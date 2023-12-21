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

    void init(VulkanAdapter* const adapter,
        TextureManager* const textureManager,
        LightManager* const lightManager) override;

    protected:
        VulkanAdapter* m_Adapter;
        TextureManager* m_TextureManager;
        LightManager* m_LightManager;
    };
}