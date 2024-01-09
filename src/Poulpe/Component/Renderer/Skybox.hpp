#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Skybox : public AbstractRenderer
    {
    public:
        Skybox() = default;
        ~Skybox() override = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(std::chrono::duration<float> deltaTime, IVisitable* const mesh) override;
    };
}
