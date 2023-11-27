#pragma once

namespace Poulpe
{
    class Mesh;

    class IVisitor
    {
    public:
        virtual ~IVisitor() = default;
        virtual void visit(float const deltaTime, Mesh* mesh) = 0;
    };
}
