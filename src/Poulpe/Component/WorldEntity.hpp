#pragma once

#include "Entity.hpp"

namespace Poulpe
{
    class WorldEntity : public Entity
    {

    public:
        WorldEntity() {
            setName("_PlpWorldEntity");
            setVisible(false);
        };

        ~WorldEntity() = default;

        bool isDirty() override { return false; }
    };
}
