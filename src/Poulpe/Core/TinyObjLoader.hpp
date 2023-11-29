#pragma once

#include "Poulpe/Component/Vertex.hpp"

namespace Poulpe
{
    struct TinyObjData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        uint32_t materialId = 0;
        std::vector<uint32_t> facesMaterialId;
    };

    struct material_t {
      std::string name;

      glm::vec3 ambient;
      glm::vec3 diffuse;
      glm::vec3 specular;
      glm::vec3 transmittance;
      glm::vec3 emission;
      float shininess;
      float ior;       // index of refraction
      float dissolve;  // 1 == opaque; 0 == fully transparent
      std::string ambientTexname;             // map_Ka
      std::string diffuseTexname;             // map_Kd
      std::string specularTexname;            // map_Ks
      std::string specularHighlightTexname;  // map_Ns
      std::string bumpTexname;                // map_bump, map_Bump, bump
    };

    class TinyObjLoader

    {
    public:
        static std::vector<TinyObjData> loadData(std::string const & path, bool shouldInverseTextureY);

        static std::vector<Poulpe::material_t> m_TinyObjMaterials;
    };
}
