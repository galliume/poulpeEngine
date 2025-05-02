#include "AssimpLoader.hpp"
#include "AssimpLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <assimp/GltfMaterial.h>

namespace Poulpe
{
  //helper from learnopengl
  static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
  {
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
  }

  static inline glm::vec3 GetGLMVec(const aiVector3D& vec)
  {
    return glm::vec3(vec.x, vec.y, vec.z);
  }

  static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
  {
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
  }
  //

  void AssimpLoader::loadData(
        std::string const & path,
        bool const flip_Y,
        std::function<void(
          PlpMeshData const _data,
          std::vector<material_t> const materials,
          bool const exists,
          std::vector<Animation> const animations,
          std::unordered_map<std::string, std::vector<std::vector<Position>>> const positions,
          std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const rotations,
          std::unordered_map<std::string, std::vector<std::vector<Scale>>> const scales)> callback)
  {
    Assimp::Importer importer;


    auto flags {
        aiProcess_Triangulate
      | aiProcess_OptimizeMeshes
      | aiProcess_GenNormals
      | aiProcess_CalcTangentSpace
      | aiProcess_MakeLeftHanded
      | aiProcess_FlipWindingOrder
      | aiProcess_FlipUVs
    };

    PLP_DEBUG("Loading {}", path);

    const aiScene* scene = importer.ReadFile(path, flags);

    if (nullptr == scene) {
      PLP_ERROR("Error while importing file {}: {}", path, importer.GetErrorString());
      return;
    }

    std::vector<material_t> materials{};
    materials.reserve(scene->mNumMaterials);

    if (scene->HasMaterials()) {
      for (auto i{ 0 }; i < scene->mNumMaterials; ++i) {

        auto const& mat = scene->mMaterials[i];

        material_t material{};

        aiString texture_file;
        mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_file);
        if(auto texture = scene->GetEmbeddedTexture(texture_file.C_Str())) {
          //returned pointer is not null, read texture from memory
        }
        //PLP_DEBUG("texture_file {}", texture_file.C_Str());
        material.name = mat->GetName().C_Str();

        aiColor4D baseColor(1.f);
        if (mat->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS) {
          material.base_color = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
        }
        aiColor4D ambientColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == aiReturn_SUCCESS) {
          material.ambient = { ambientColor.r, ambientColor.g, ambientColor.b, ambientColor.a };
        }
        aiColor4D diffuseColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS) {
          material.diffuse = { diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a };
        }
        aiColor4D specularColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == aiReturn_SUCCESS) {
          material.specular = { specularColor.r, specularColor.g, specularColor.b, specularColor.a };
        }
        aiColor3D transmittanceColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_TRANSPARENT, transmittanceColor) == aiReturn_SUCCESS) {
          material.transmittance = { transmittanceColor.r, transmittanceColor.g, transmittanceColor.b };
        }
        float e_factor;
        if (mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, e_factor) == aiReturn_SUCCESS) {
          material.mre_factor.z = e_factor;
        }
        float ior{ 1.f };
        if (mat->Get(AI_MATKEY_REFRACTI, ior) == aiReturn_SUCCESS) {
          material.ior = ior;
        }
        float dissolve{ 1.f };
        if (mat->Get(AI_MATKEY_OPACITY, dissolve) == aiReturn_SUCCESS) {
          material.dissolve = dissolve;
        }

        aiString alpha_mode;
        aiString mask{ "MASK" };
        aiString opaque{ "OPAQUE" };
        aiString blend{ "BLEND" };
        material.alpha_mode = 0.0f;//OPAQUE
        if (mat->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode) == aiReturn_SUCCESS) {
          float mode{ 0.0f };//OPAQUE by default

          if (alpha_mode == mask) mode = 1.0f;
          else if (alpha_mode == blend) mode = 2.0f;

          material.alpha_mode = mode;
        }
        float alpha_cut_off;
        if (mat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alpha_cut_off) == aiReturn_SUCCESS) {
          material.alpha_cut_off = alpha_cut_off;
        }
        float m;
        if (mat->Get(AI_MATKEY_METALLIC_FACTOR, m) == aiReturn_SUCCESS) {
          material.mre_factor.x = m;
        }
        float r;
        if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, r) == aiReturn_SUCCESS) {
          material.mre_factor.y = r;
        }
        float transmission_strength{ 1.0 };
        if (mat->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmission_strength) == aiReturn_SUCCESS) {
          material.transmission_strength = transmission_strength;
        }
        float ao_strength{ 1.0 };
        if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_LIGHTMAP, 0), ao_strength) == aiReturn_SUCCESS) {
          material.occlusion_strength = ao_strength;
        }
        float glossiness{ 1.0 };
        if (mat->Get(AI_MATKEY_SHININESS_STRENGTH, glossiness) == aiReturn_SUCCESS) {
          material.shininess = glossiness;
        }
        

        if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_AMBIENT, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_ambient = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_AMBIENT, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_ambient_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_AMBIENT, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_ambient_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
          aiUVTransform transform{};
          if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_AMBIENT, 0), transform) == aiReturn_SUCCESS) {
             material.ambient_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
             material.ambient_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
             material.ambient_rotation = glm::vec2(transform.mRotation, 1.0);
          }
        }
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_diffuse = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_DIFFUSE, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_diffuse_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_DIFFUSE, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_diffuse_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
          aiUVTransform transform{};
          if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, 0), transform) == aiReturn_SUCCESS) {
            material.diffuse_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
            material.diffuse_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
            material.diffuse_rotation = glm::vec2(transform.mRotation, 1.0);
          }
        }
        if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_SPECULAR, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_specular = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_SPECULAR, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_specular_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_SPECULAR, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_specular_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
        }
        if (mat->GetTextureCount(aiTextureType_SHININESS) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_SHININESS, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_specular_highlight = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_SHININESS, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_specular_highlight_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_SHININESS, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_specular_highlight_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
        }

        bool const is_obj {std::filesystem::path(path).extension() == ".obj"};

        if (is_obj) {
          if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {
            aiString texture_path;
            aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
            aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

            if (mat->GetTexture(aiTextureType_HEIGHT, 0, &texture_path) == aiReturn_SUCCESS) {
                material.name_texture_bump = AssimpLoader::cleanName(texture_path.C_Str());
            }
            if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_HEIGHT, 0), wrap_mode_u) == aiReturn_SUCCESS) {
              material.texture_bump_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
            }
            if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_HEIGHT, 0), wrap_mode_v) == aiReturn_SUCCESS) {
              material.texture_bump_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
            }
            aiUVTransform transform{};
            if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_HEIGHT, 0), transform) == aiReturn_SUCCESS) {
              material.normal_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
              material.normal_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
              material.normal_rotation = glm::vec2(transform.mRotation, 1.0);
            }
          }
        } else {
          if (mat->GetTextureCount(aiTextureType_NORMALS) > 0) {
            aiString texture_path;
            aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
            aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

            if (mat->GetTexture(aiTextureType_NORMALS, 0, &texture_path) == aiReturn_SUCCESS) {
                material.name_texture_bump = AssimpLoader::cleanName(texture_path.C_Str());
            }
            if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_NORMALS, 0), wrap_mode_u) == aiReturn_SUCCESS) {
              material.texture_bump_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
            }
            if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_NORMALS, 0), wrap_mode_v) == aiReturn_SUCCESS) {
              material.texture_bump_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
            }
            aiUVTransform transform{};
            if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_NORMALS, 0), transform) == aiReturn_SUCCESS) {
              material.normal_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
              material.normal_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
              material.normal_rotation = glm::vec2(transform.mRotation, 1.0);
            }
            float strength{ 1.0 };
            //if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_NORMALS, 0), strength) == aiReturn_SUCCESS) {
            //  material.normal_strength = strength;
            //}
          }
        }
        if (mat->GetTextureCount(aiTextureType_OPACITY) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_OPACITY, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_alpha = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_OPACITY, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_alpha_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_OPACITY, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_alpha_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
        }
        if (mat->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_UNKNOWN, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_metal_roughness = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_UNKNOWN, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_metal_roughness_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_UNKNOWN, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_metal_roughness_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
          aiUVTransform transform{};
          if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_UNKNOWN, 0), transform) == aiReturn_SUCCESS) {
            material.mr_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
            material.mr_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
            material.mr_rotation = glm::vec2(transform.mRotation, 1.0);
          }
        }
        if (mat->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };
          
          if (mat->GetTexture(aiTextureType_EMISSIVE, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_emissive = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_EMISSIVE, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_emissive_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_EMISSIVE, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_emissive_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
          aiUVTransform transform{};
          if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_EMISSIVE, 0), transform) == aiReturn_SUCCESS) {
            material.emissive_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
            material.emissive_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
            material.emissive_rotation = glm::vec2(transform.mRotation, 1.0);
          }
        }
        if (mat->GetTextureCount(aiTextureType_LIGHTMAP) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_LIGHTMAP, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_ao = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_LIGHTMAP, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_ao_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_LIGHTMAP, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_ao_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
        }

        if (mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };

          if (mat->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_base_color = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_BASE_COLOR, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_base_color_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_BASE_COLOR, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_base_color_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
        }
        if (mat->GetTextureCount(aiTextureType_TRANSMISSION) > 0) {
          aiString texture_path;
          aiTextureMapMode wrap_mode_u { aiTextureMapMode_Clamp };
          aiTextureMapMode wrap_mode_v { aiTextureMapMode_Clamp };
          
          if (mat->GetTexture(aiTextureType_TRANSMISSION, 0, &texture_path) == aiReturn_SUCCESS) {
              material.name_texture_transmission = AssimpLoader::cleanName(texture_path.C_Str());
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_U(aiTextureType_TRANSMISSION, 0), wrap_mode_u) == aiReturn_SUCCESS) {
            material.texture_transmission_wrap_mode_u = getTextureWrapMode(wrap_mode_u);
          }
          if (mat->Get(AI_MATKEY_MAPPINGMODE_V(aiTextureType_TRANSMISSION, 0), wrap_mode_v) == aiReturn_SUCCESS) {
            material.texture_transmission_wrap_mode_v = getTextureWrapMode(wrap_mode_v);
          }
          aiUVTransform transform{};
          if (mat->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_TRANSMISSION, 0), transform) == aiReturn_SUCCESS) {
            material.transmission_translation = glm::vec3(transform.mTranslation.x, transform.mTranslation.y, 1.0);
            material.transmission_scale = glm::vec3(transform.mScaling.x, transform.mScaling.y, 1.0);
            material.transmission_rotation = glm::vec2(transform.mRotation, 1.0);
          }
        }


        //int illumModel;
        //if (mat->Get(AI_MATKEY_ILLUM, illumModel) == aiReturn_SUCCESS) {
        //  material.illum = material.illum;
        //}
        int isDoubleSided = 0;
        if (mat->Get(AI_MATKEY_TWOSIDED, isDoubleSided) == aiReturn_SUCCESS) {
          material.double_sided = (isDoubleSided == 1) ? true : false;
        }

        materials.emplace_back(material);
      }
    }

    std::vector<Animation> animations{};
    std::unordered_map<std::string, std::vector<std::vector<Rotation>>> rotations{};
    std::unordered_map<std::string, std::vector<std::vector<Position>>> positions{};
    std::unordered_map<std::string, std::vector<std::vector<Scale>>> scales{};

    animations.reserve(scene->mNumAnimations);

    for (unsigned int i{ 0 }; i < scene->mNumAnimations; i++) {

      aiAnimation const* animation = scene->mAnimations[i];

      auto const ticks_per_s = (animation->mTicksPerSecond > 0) ? animation->mTicksPerSecond : 25.0;
      animations.emplace_back(i, animation->mName.C_Str(), animation->mDuration, ticks_per_s);

      //PLP_DEBUG("Animation {}, duration {}", animation->mName.C_Str(), animation->mDuration);
      for (unsigned int j{ 0 }; j < animation->mNumChannels; j++) {
        aiNodeAnim const* node = animation->mChannels[j];
        std::string const node_name{ node->mNodeName.C_Str() };
        rotations[node_name].reserve(scene->mNumAnimations);
        positions[node_name].reserve(scene->mNumAnimations);
        scales[node_name].reserve(scene->mNumAnimations);

        std::vector<Rotation>rots{};
        rots.reserve(node->mNumRotationKeys);
        unsigned int id{ 0 };
        for (unsigned int r{ 0 }; r < node->mNumRotationKeys; r++) {
          aiQuatKey const& rotation_key = node->mRotationKeys[r];
          auto const interpolation{ getInterpolation(rotation_key.mInterpolation) };
          //PLP_DEBUG("rot {} x {} y {} z {}", rotation_key.mTime, rotation_key.mValue.x, rotation_key.mValue.y, rotation_key.mValue.z);
          rots.emplace_back(Rotation{ id, i, static_cast<float>(rotation_key.mTime), interpolation, GetGLMQuat(rotation_key.mValue) });
          id += 1;
        }
        //auto rot_duplicate = rots.front();
        //rot_duplicate.id = rots.size() + 1;
        //rot_duplicate.time = animation->mDuration;
        //rots.emplace_back(rot_duplicate);
        rotations[node_name].push_back(rots);

        id = 0;
        std::vector<Position> pos{};
        pos.reserve(node->mNumPositionKeys + 1);
        for (unsigned int p{ 0 }; p < node->mNumPositionKeys; p++) {
          aiVectorKey const& pos_key = node->mPositionKeys[p];
          auto const interpolation{ getInterpolation(pos_key.mInterpolation) };

          //PLP_DEBUG("pos {} x {} y {} z {}", pos_key.mTime, pos_key.mValue.x, pos_key.mValue.y, pos_key.mValue.z);
          pos.emplace_back(Position{ id, i, static_cast<float>(pos_key.mTime), interpolation, GetGLMVec(pos_key.mValue) });
          id += 1;
        }
        //auto pos_duplicate = pos.front();
        //pos_duplicate.id = pos.size() + 1;
        //pos_duplicate.time = animation->mDuration;
        //pos.emplace_back(pos_duplicate);
        positions[node_name].push_back(pos);

        id = 0;
        std::vector<Scale> sc{};
        sc.reserve(node->mNumScalingKeys);
        for (unsigned int s{ 0 }; s < node->mNumScalingKeys; s++) {
          aiVectorKey const& scale_key = node->mScalingKeys[s];
          auto const interpolation{ getInterpolation(scale_key.mInterpolation) };

          //PLP_DEBUG("scale {} x {} y {} z {}", scale_key.mTime, scale_key.mValue.x, scale_key.mValue.y, scale_key.mValue.z);
          sc.emplace_back(Scale{ id, i, static_cast<float>(scale_key.mTime), interpolation, GetGLMVec(scale_key.mValue) });
          id += 1;
        }
        //auto sc_duplicate = sc.front();
        //sc_duplicate.id = sc.size() + 1;
        //sc_duplicate.time = animation->mDuration;
        //sc.emplace_back(sc_duplicate);
        scales[node_name].push_back(sc);
      }
    }

    glm::mat4 const global_transform = ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation);

    std::vector<PlpMeshData> mesh_data{};
    process(scene->mRootNode, scene, mesh_data, global_transform, flip_Y);

    size_t id{ mesh_data.size() };
    for (auto& data : mesh_data) {
      --id;
      data.id = id;
      callback(data, materials, false, animations, positions, rotations, scales);
    }
  }

  std::string const AssimpLoader::cleanName(std::string const & name)
  {
    std::string cleaned{};

    if (name.size() > 0) {
      size_t lastindex = name.find_last_of(".");
      cleaned = name.substr(0, lastindex);

      std::replace(cleaned.begin(), cleaned.end(), '\\', '_');
      std::replace(cleaned.begin(), cleaned.end(), '/', '_');
    }

    //PLP_DEBUG("asset: {} -> {}", name, cleaned);

    return cleaned;
  }

  void AssimpLoader::process(
    aiNode* node,
    const aiScene *scene,
    std::vector<PlpMeshData>& data,
    glm::mat4 const& global_transform,
    bool const flip_Y)
  {
    glm::mat4 local_transform = ConvertMatrixToGLMFormat(node->mTransformation);
    auto transform_matrix = global_transform * local_transform;

    for (unsigned int i{ 0 }; i < node->mNumMeshes; i++) {
      PlpMeshData mesh_data{};
      mesh_data.transform_matrix = transform_matrix;
      mesh_data.inverse_transform_matrix = glm::inverse(global_transform);

      aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];
      mesh_data.name = mesh->mName.C_Str() + std::to_string(i);
      unsigned int count{ 0 };

      //@todo check if it's ok
      //fallback to last normal or tangent if none is found
      //gives some artifacts but better than nothing
      glm::vec3 n{ 0.5f, 0.5f, 1.0f };
      glm::vec4 t{ 0.5f, 0.5f, 1.0f, 1.0f };

      mesh_data.vertices.reserve(mesh->mNumVertices);

      for (unsigned int v{ 0 }; v < mesh->mNumVertices; v++) {

        aiVector3D vertices = mesh->mVertices[v];

        Vertex vertex{};
        vertex.bone_ids.resize(4, 0);
        vertex.bone_weights.resize(4, 0.0f);

        vertex.pos = { vertices.x, vertices.y, vertices.z };
        if (flip_Y) vertex.pos.y *= -1.0f;
        vertex.original_pos = vertex.pos;

        vertex.normal = n;

        if (mesh->HasNormals()) {
          aiVector3D const& normal = mesh->mNormals[v];
          vertex.normal = { normal.x, normal.y, normal.z };
          n = vertex.normal;
          //if (flip_Y) vertex.normal.y = 1.0f - vertex.normal.y;
        } else {
          PLP_WARN("NO NORMAL");
        }

        glm::vec4 tangent(0.f);
        glm::vec4 bitangent(0.f);

        if (mesh->HasTangentsAndBitangents()) {
          tangent.x += mesh->mTangents[v].x;
          tangent.y += mesh->mTangents[v].y;
          tangent.z += mesh->mTangents[v].z;

          bitangent = glm::vec4(
            mesh->mBitangents[v].x,
            mesh->mBitangents[v].y,
            mesh->mBitangents[v].z,
            0.0f);

          //handedness
          tangent.w = glm::dot(
            glm::cross(
              glm::vec3(tangent.x, tangent.y, tangent.z),
              glm::vec3(bitangent.x, bitangent.y, bitangent.z)),
            vertex.normal) < 0.0f ? -1.0f : 1.0f;
            t = tangent;
        } 
        vertex.tangent = tangent;
        //vertex.bitangent = bitangent;

        if (mesh->mNumUVComponents[0] > 0) {
          aiVector3D texture_coord = mesh->mTextureCoords[0][v];
          vertex.texture_coord = { texture_coord.x, texture_coord.y };
          //if (flip_Y) vertex.texture_coord.y *= -1.0f;
        }

        glm::vec4 color{ 0.0f };

        auto nb_colors{ 0 };
        for (auto i{ 0 }; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++) {
          if (mesh->HasVertexColors(i)) {
            auto const& v_color = mesh->mColors[i][v];
              color += glm::vec4(
                v_color.r,
                v_color.g,
                v_color.b,
                v_color.a);
              nb_colors += 1;
          }
        }

        if (nb_colors == 0) {
          color = glm::vec4(1.0f);
        } else {
          color /= static_cast<float>(nb_colors);
        }
        
        vertex.color = color;

        mesh_data.vertices.emplace_back(std::move(vertex));
      }

      for (unsigned int f{ 0 }; f < mesh->mNumFaces; f++) {
        aiFace const* face = &mesh->mFaces[f];

        mesh_data.indices.reserve(face->mNumIndices);

        for (unsigned int j{ 0 }; j < face->mNumIndices; j++) {
          mesh_data.indices.push_back(face->mIndices[j]);
        }
        mesh_data.material_ID = mesh->mMaterialIndex;
        mesh_data.materials_ID = { mesh->mMaterialIndex };
      }

      std::unordered_map<std::string, Bone> bones_map{};

      if (mesh->HasBones()) {

        for (auto i{ 0 }; i < mesh->mNumBones; ++i) {
          auto const& bone_name = mesh->mBones[i]->mName.C_Str();
          auto* node = scene->mRootNode->FindNode(bone_name);

          while (node && bones_map.contains(node->mName.C_Str())) {
              node = node->mParent;
          }

          if (node && mesh_data.root_bone_name.empty()) {
            mesh_data.root_bone_name = node->mChildren[0]->mName.C_Str();
          }
        }

        std::unordered_map<unsigned int, std::vector<std::pair<unsigned int, float>>> vertex_weight_map{};

        for (auto b{ 1 }; b < mesh->mNumBones; b++) {
          aiBone const* bone = mesh->mBones[b];

          std::string const& bone_name{ bone->mName.C_Str() };
          aiNode const* bone_node = scene->mRootNode->FindNode(bone_name.c_str());

          auto const bone_id{ b - 1 };

          Bone bone_data{};
          bone_data.id = bone_id;
          bone_data.transform = ConvertMatrixToGLMFormat(bone_node->mTransformation);
          bone_data.offset_matrix = ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
          bone_data.name = bone_name;
          
          glm::mat4 t_pose = glm::mat4(1.0f);
          aiNode const* current = bone_node;

          while (current) {
            t_pose = ConvertMatrixToGLMFormat(current->mTransformation) * t_pose;
            current = current->mParent;
          }
          bone_data.t_pose = t_pose;

          for (auto w{ 0 }; w < bone->mNumWeights; w++) {
            aiVertexWeight const& aiWeight = bone->mWeights[w];
            bone_data.weights.emplace_back(aiWeight.mVertexId, aiWeight.mWeight);
            
            auto& data = vertex_weight_map[aiWeight.mVertexId];
            data.emplace_back(bone_id, aiWeight.mWeight);
          }

          for (auto i{ 0 }; i < bone_node->mNumChildren; i++) {
            aiNode* child = bone_node->mChildren[i];
            if (child) {
              std::string const& child_name{ child->mName.C_Str() };
              bone_data.children.emplace_back(child_name);
            }
          }
          bones_map[bone_data.name] = std::move(bone_data);
        }
        mesh_data.bones = bones_map;

        for (auto& vertex_map : vertex_weight_map) {
          unsigned int vertex_id = vertex_map.first;
          auto& vertex = mesh_data.vertices.at(vertex_id);

          auto& data{ vertex_map.second };

          std::sort(data.begin(), data.end(),
            [](auto const& a, auto const& b) { return a.second > b.second; });
            
          float total_weight{ 0.0f };
          for (int i{ 0 }; i < 4 && i < static_cast<int>(data.size()); ++i) {
            vertex.bone_ids[i] = data[i].first;
            vertex.bone_weights[i] = data[i].second;
            total_weight += data[i].second;
          }

          //if (total_weight > 0.0f) {
          //  for (int i{ 0 }; i < 4; ++i)
          //  vertex.bone_weights[i] /= total_weight;
          //}
        }
      }
      data.emplace_back(mesh_data);
    }

    for (unsigned int i{ 0 }; i < node->mNumChildren; i++) {
      process(node->mChildren[i], scene, data, global_transform, flip_Y);
    }
  }

  TextureWrapMode AssimpLoader::getTextureWrapMode(aiTextureMapMode const wrap_mode)
  {
    switch (wrap_mode)
    {
    case 1:
      return TextureWrapMode::CLAMP_TO_EDGE;
    case 2:
      return TextureWrapMode::MIRROR_REPEAT;
      break;
    case 0:
    default:
      return TextureWrapMode::WRAP;
      break;
    }
  }
  
  AnimInterpolation AssimpLoader::getInterpolation(aiAnimInterpolation const assimp_interpolation)
  {
    AnimInterpolation interpolation{ AnimInterpolation::STEP };
    switch (assimp_interpolation) {
    case aiAnimInterpolation_Linear :
      interpolation = AnimInterpolation::LINEAR;
      break;
    case aiAnimInterpolation_Spherical_Linear :
      interpolation = AnimInterpolation::SPHERICAL_LINEAR;
      break;
      case aiAnimInterpolation_Cubic_Spline:
        interpolation = AnimInterpolation::CUBIC_SPLINE;
    }
    return interpolation;
  }
}
