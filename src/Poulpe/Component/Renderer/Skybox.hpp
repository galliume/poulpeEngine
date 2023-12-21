#pragma once

#include "AbstractEntity.hpp"

namespace Poulpe
{
    class Skybox : public AbstractRenderer
    {
    public:
        Skybox() = default;
        ~Skybox() = default;

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;
        void createDescriptorSet(Mesh* mesh) override;
    };
}
