#pragma once

#include "AbstractEntity.hpp"

namespace Poulpe
{
    class Grid : public AbstractRenderer
    {
    public:
        Grid() = default;
        ~Grid() = default;

        void visit(float const deltaTime, Mesh* mesh) override;
        void setPushConstants(Mesh* mesh) override;
        void createDescriptorSet(Mesh* mesh) override;
    };
}
