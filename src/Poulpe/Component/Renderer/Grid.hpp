#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Grid : public AbstractRenderer
    {
    public:
        Grid() = default;
        ~Grid() override = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(std::chrono::duration<float> deltaTime, IVisitable* const mesh) override;
    };
}
