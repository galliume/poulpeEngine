#pragma once

#include "PlpTypedef.hpp"

#include <assimp/config.h>
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
        std::unordered_map<std::string, std::vector<std::vector<Poulpe::Position>>> const positions,
        std::unordered_map<std::string, std::vector<std::vector<Poulpe::Rotation>>> const rotations,
        std::unordered_map<std::string, std::vector<std::vector<Poulpe::Scale>>> const scales)> callback);

  private:
     static std::string const cleanName(std::string const& name, std::string const& prefix);
     static void process(aiNode* node,
       const aiScene *scene,
       std::vector<PlpMeshData>& mesh_data,
       glm::mat4 const& global_transform,
       std::string const& texture_prefix,
       bool const flip_Y);
    static TextureWrapMode getTextureWrapMode(aiTextureMapMode const wrap_mode);
    static AnimInterpolation getInterpolation(aiAnimInterpolation const assimp_interpolation);
    static aiNode const* FindRootBone(
      aiNode const* node,
      std::unordered_set<std::string> const& bone_names);
  };
}
