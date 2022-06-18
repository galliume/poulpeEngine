#pragma once

#include "Drawable.h"

namespace Rbk
{
    class Entity
    {

    public:
        virtual void Draw();
        virtual ~Entity();

    protected:
        std::unique_ptr<Drawable> m_Drawable;
    };
}