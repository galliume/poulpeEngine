#pragma once
#include "Rebulk/Component/Mesh.hpp"

namespace Rbk
{
    struct TinyObjData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialId = 0;
    };

    class TinyObjLoader
    {
    public:
        static std::vector<TinyObjData> LoadData(std::string path, bool shouldInverseTextureY);
    };
}
