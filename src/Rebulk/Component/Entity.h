#pragma once

#include "Drawable.h"

namespace Rbk
{
    class Entity
    {

    public:
        virtual void Draw();
        virtual ~Entity();

    //@todo make it private
    public:
        std::string m_Name;

    protected:
        std::unique_ptr<Drawable> m_Drawable;

    };
}