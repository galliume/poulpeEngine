#pragma once

#include "Drawable.h"

namespace Rbk
{
    class Entity
    {
    public:
        std::unique_ptr<Drawable> m_Drawable;
    };
}