#pragma once
#include "Rebulk/Renderer/Mesh.h"

namespace Rbk
{
    class TinyObjLoader
    {
    public:
        static std::shared_ptr<Rbk::Mesh> LoadMesh(const char* path, bool shouldInverseTextureY);
    };
}
