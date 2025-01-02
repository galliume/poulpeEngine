#pragma once

#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Component/Vertex.hpp"

namespace Poulpe
{
    struct TinyObjData
    {
        uint32_t material_ID = 0;
        std::string name;
        std::vector<uint32_t> face_material_ID;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> materials_ID;
        std::vector<Vertex> vertices;
    };

    class TinyObjLoader

    {
    public:
      void static loadData(
        std::string const & path,
        bool const flip_Y,
        std::function<void(
          TinyObjData const& _data,
          std::vector<material_t> const& materials,
          bool const exists)> callback);

    private:
      static std::string const cleanName(std::string const& name);
    };
}
