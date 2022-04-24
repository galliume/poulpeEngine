#pragma once
#include "Rebulk/Renderer/Mesh.h"

namespace Rbk
{
    class TinyObjLoader
    {
    public:
        static Mesh LoadMesh(const char* path, bool shouldInverseTextureY);
    };
}
