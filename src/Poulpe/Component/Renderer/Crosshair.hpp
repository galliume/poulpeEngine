#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Crosshair : public AbstractRenderer
    {
    public:
        Crosshair() = default;

        void visit(float const deltaTime, IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void createDescriptorSet(IVisitable* const mesh) override;
    };
}