#pragma once

#include "Poulpe/Utils/IDHelper.hpp"

namespace Poulpe
{
    class Entity;

    class Component
    {
    public:
        Component() {
            m_ID = GUIDGenerator::getGUID();
        }
        virtual ~Component() {}
        //virtual void Initialize() {}
        //virtual void Update([[maybe_unused]] float deltaTime) {}
        //virtual void Render() {}

        void setOwner(IDType owner) { m_Owner = owner; }
        IDType getOwner() { return m_Owner; }
        IDType getID() { return m_ID; }

    private:
        IDType m_Owner;
        IDType m_ID;
    };
}