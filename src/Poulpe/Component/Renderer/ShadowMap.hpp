#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class ShadowMap : public AbstractRenderer
    {
    public:
        ShadowMap() = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(std::chrono::duration<float> deltaTime, IVisitable* const visitable) override;
    };
}
