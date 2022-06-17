#pragma once
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class TinyObjLoader
    {
    public:
        static std::vector<std::shared_ptr<Mesh>> LoadMesh(std::string path, bool shouldInverseTextureY);
    };
}
