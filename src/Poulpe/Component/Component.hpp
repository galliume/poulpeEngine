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

        template<typename... TArgs>
        void visit(float deltaTime, TArgs&&... args)
        {
            m_Pimpl->visit(deltaTime, std::forward<TArgs>(args)...);
        }

        template<typename T>
        T* hasImpl() { return dynamic_cast<T*>(m_Pimpl); }

        void setOwner(IDType owner) { m_Owner = owner; }

        IDType getID() { return m_ID; }
        IDType getOwner() { return m_Owner; }

    protected:
        IVisitor* m_Pimpl;

    private:
        IDType m_ID;
        IDType m_Owner;

        //@todo make unique_ptr ?
    };
}