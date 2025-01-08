#pragma once

#include "PlpTypedef.hpp"

#include <assimp/scene.h>

namespace Poulpe
{

  class AssimpLoader

  {
  public:
    void static loadData(
      std::string const & path,
      bool const flip_Y,
      std::function<void(
        PlpMeshData const _data,
        std::vector<material_t> const materials,
        bool const exists,
        std::vector<Animation> const animations,
        std::vector<Position> const positions,
        std::vector<Rotation> const rotations,
        std::vector<Scale> const scales)> callback);

  private:
     static std::string const cleanName(std::string const& name);
     static void process(aiNode* node,
       const aiScene *scene,
       std::vector<PlpMeshData>& mesh_data,
       bool const flip_Y);
    static TextureWrapMode AssimpLoader::getTextureWrapMode(aiTextureMapMode const wrap_mode);
  };
}
