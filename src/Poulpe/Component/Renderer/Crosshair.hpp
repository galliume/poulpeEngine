#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Crosshair : public AbstractRenderer
    {
    public:
        Crosshair() = default;
        ~Crosshair() = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(std::chrono::duration<float> deltaTime, IVisitable* const mesh) override;
    };
}