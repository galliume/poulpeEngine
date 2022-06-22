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

    //@todo make it private
    public:
        std::string m_Name;


    };
}