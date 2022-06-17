#pragma once
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    struct Data;

    class TinyObjLoader
    {
    public:
        static std::vector<Data> LoadData(std::string path, bool shouldInverseTextureY);
    };
}
