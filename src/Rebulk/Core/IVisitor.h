#pragma once
#include <memory>

#include "Rebulk/Component/Drawable.h"

namespace Rbk
{
    class Drawable;

    class IVisitor
    {
    public:
        //@todo make it weak ptr
        virtual void VisitDrawable(Drawable* drawable) = 0;
    };
}