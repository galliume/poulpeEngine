module;


#include <assimp/config.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

export module Engine.Core.AssimpLoader;

import std;

import Engine.Component.Vertex;
import Engine.Core.PlpTypedef;
import Engine.Core.Logger;

namespace Poulpe
{
  export class AssimpLoader
  {
  public:
    void static loadData(
      std::string const & path,
      bool const flip_Y,
      std::function<void(
        PlpMeshData const _data,
        std::vector<material_t> const materials,
        std::vector<Animation> const animations,
        std::unordered_map<std::string, std::vector<std::vector<Position>>> const positions,
        std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const rotations,
        std::unordered_map<std::string, std::vector<std::vector<Scale>>> const scales)> callback);

  private:
      static void getBoneHierarchy(aiNode* node, std::unordered_set<std::string>& bone_names);
      static std::string const cleanName(std::string const& name, std::string const& prefix);
      static void addWeightlessBones(aiNode const * node, std::unordered_map<std::string, Bone>& bones_map, uint32_t global_bone_count);
      static void process(
        std::string const& root_bone,
        aiNode* node,
        const aiScene *scene,
        std::vector<PlpMeshData>& mesh_data,
        glm::mat4 const& global_transform,
        glm::mat4 const& inverse_global_transform,
        std::string const& texture_prefix,
        bool const flip_Y,
        std::unordered_map<std::string, Bone> & bones_map,
        uint32_t global_bone_count);
    static TextureWrapMode getTextureWrapMode(aiTextureMapMode const wrap_mode);
    static AnimInterpolation getInterpolation(aiAnimInterpolation const assimp_interpolation);
    static aiNode const* FindRootBone(
      aiScene const * scene,
      aiNode const* node,
      std::unordered_set<std::string> const& bone_names);
  };
}
