#pragma once

namespace Poulpe
{
    class Object
    {
    public:
        virtual void accept(std::weak_ptr<Poulpe::IVisitor> visitor) {}
    };
}