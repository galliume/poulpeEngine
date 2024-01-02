#pragma once

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Utils/IDHelper.hpp"

namespace Poulpe
{
    class Entity;

    class Component
    {
    public:
        Component() { m_ID = GUIDGenerator::getGUID(); }
        ~Component() { delete m_Pimpl; }

        IDType getID() { return m_ID; }
        IDType getOwner() { return m_Owner; }

        template<typename T>
        T* hasImpl() { return dynamic_cast<T*>(m_Pimpl); }

        template<std::derived_from<IVisitor> T>
        void init(T* componentImpl) 
        {
            m_Pimpl = std::move(componentImpl);
        }

        template<typename... TArgs>
        void initPimpl(TArgs&&... args)
        {
            m_Pimpl->init(std::forward<TArgs>(args)...);
        }

        void setOwner(IDType owner) { m_Owner = owner; }

        template<typename... TArgs>
        void visit(std::chrono::duration<float> deltaTime, TArgs&&... args)
        {
            m_Pimpl->visit(deltaTime, std::forward<TArgs>(args)...);
        }

    protected:
        IVisitor* m_Pimpl;

    private:
        IDType m_ID;
        IDType m_Owner;
    };
}