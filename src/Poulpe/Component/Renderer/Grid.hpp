#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Grid : public AbstractRenderer
    {
    public:
        Grid() = default;
        ~Grid() = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(float const deltaTime, IVisitable* const mesh) override;
    };
}
