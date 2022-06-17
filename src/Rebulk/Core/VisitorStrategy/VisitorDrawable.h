#pragma once

#include "Rebulk/Component/Drawable.h"

namespace Rbk
{
    class VisitorDrawable
    {
    public:
        void VisitDrawable(Rbk::Drawable* drawable);
    };
}