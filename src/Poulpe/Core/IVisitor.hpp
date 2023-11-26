#pragma once

namespace Poulpe
{
    class Mesh;

    class IVisitor
    {
    public:
        IVisitor() = default;
        virtual ~IVisitor() = 0;

        virtual void visit(Mesh* mesh) = 0;
    };
}
