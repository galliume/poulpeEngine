#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Basic : public AbstractRenderer
    {
    public:
        Basic() = default;

        void visit(float const deltaTime, IVisitable* const visitable) override;
        void setPushConstants(IVisitable* const mesh) override;
        void createDescriptorSet(IVisitable* const mesh) override;
    };
}
