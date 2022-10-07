#pragma once

#include "Mesh.h"

namespace Rbk
{
    //@todo should be reversed and Mesh rename to Mesh3D...
    class Mesh2D : public Mesh
    {
    public:
        std::vector<Vertex2D> m_Vertices;
    };
}