#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Basic : public AbstractRenderer
    {
    public:
        Basic() = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(std::chrono::duration<float> deltaTime, IVisitable* const visitable) override;
    };
}
