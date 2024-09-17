#pragma once

#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Component/Vertex.hpp"

namespace Poulpe
{
    struct TinyObjData
    {
        uint32_t materialId = 0;
        std::string name;
        std::vector<uint32_t> facesMaterialId;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> materialsID;
        std::vector<Vertex> vertices;
    };

    class TinyObjLoader

    {
    public:
        static std::vector<TinyObjData> loadData(std::string const & path, std::vector<material_t> & tinyObjMaterials, bool shouldInverseTextureY);
    };
}
