#pragma once

#include "Poulpe/Core/IVisitor.h"

namespace Poulpe
{
    class Object
    {
    public:
        virtual void Accept(std::weak_ptr<Poulpe::IVisitor> visitor) {}
    };
}