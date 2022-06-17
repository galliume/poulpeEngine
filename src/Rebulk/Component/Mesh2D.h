#pragma once

#include "Mesh.h"

namespace Rbk
{
    class Mesh2D : public Mesh
    {
    public:
        std::vector<Vertex2D> vertices;
    };
}