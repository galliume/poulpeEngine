#pragma once

#include "AbstractRenderer.hpp"

namespace Poulpe
{
    class Skybox : public AbstractRenderer
    {
    public:
        Skybox() = default;
        ~Skybox() = default;

        void createDescriptorSet(IVisitable* const mesh) override;
        void setPushConstants(IVisitable* const mesh) override;
        void visit(float const deltaTime, IVisitable* const mesh) override;
    };
}
