#pragma once

#include "AbstractEntity.hpp"

namespace Poulpe
{
    class Crosshair : public AbstractRenderer
    {
    public:
        Crosshair() = default;

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;
        void createDescriptorSet(Mesh* mesh) override;
    };
}