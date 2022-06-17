#pragma once

#include "Rebulk/Component/Object.h"

namespace Rbk
{
    class Drawable : public Object
    {
    public:
        void Accept(std::weak_ptr<Rbk::IVisitor> visitor) override;
    };
}