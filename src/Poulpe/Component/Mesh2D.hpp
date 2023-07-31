#pragma once

#include "Mesh.hpp"

namespace Poulpe
{
    //@todo should be reversed and Mesh rename to Mesh3D...
    class Mesh2D : public Mesh
    {
    public:
        std::vector<Vertex2D> m_Vertices;
    };
}