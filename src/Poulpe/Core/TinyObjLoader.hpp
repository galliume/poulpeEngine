#pragma once

#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Component/Vertex.hpp"

namespace Poulpe
{
    struct TinyObjData
    {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialId = 0;
        std::vector<uint32_t> facesMaterialId;
        std::vector<uint32_t> materialsID;
    };

    class TinyObjLoader

    {
    public:
        static std::vector<TinyObjData> loadData(std::string const & path, bool shouldInverseTextureY);

        static std::vector<Poulpe::material_t> m_TinyObjMaterials;
    };
}
