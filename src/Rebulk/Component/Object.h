#pragma once

#include "Rebulk/Core/IVisitor.h"

namespace Rbk
{
    class Object
    {
    public:
        virtual void Accept(std::weak_ptr<Rbk::IVisitor> visitor) {}
    };
}