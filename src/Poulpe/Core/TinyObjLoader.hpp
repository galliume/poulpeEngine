#pragma once
#include "Poulpe/Component/Mesh.hpp"

namespace Poulpe
{
    struct TinyObjData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialId = 0;
        std::vector<uint32_t> facesMaterialId;
    };

    class TinyObjLoader
    {
    public:
        static std::vector<TinyObjData> loadData(std::string path, bool shouldInverseTextureY);
    };
}
