#pragma once

#include "Rebulk/Core/IVisitor.h"

namespace Rbk
{
    class Entity : public std::enable_shared_from_this<Entity>
    {

    public:
        virtual void Draw();
        virtual void Accept(std::shared_ptr<IVisitor> visitor);
        virtual ~Entity();
        
        inline const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

    private:
        std::string m_Name;
    };
}