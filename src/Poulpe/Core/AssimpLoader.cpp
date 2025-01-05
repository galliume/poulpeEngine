#include "AssimpLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

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
          std::vector<Position> const positions,
          std::vector<Rotation> const rotations,
          std::vector<Scale> const scales)> callback)
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

        aiColor3D ambientColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
          material.ambient = { ambientColor.r, ambientColor.g, ambientColor.b };
        }
        aiColor3D diffuseColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
          material.diffuse = { diffuseColor.r, diffuseColor.g, diffuseColor.b };
        }
        aiColor3D specularColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
          material.specular = { specularColor.r, specularColor.g, specularColor.b };
        }
        aiColor3D transmittanceColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_TRANSPARENT, transmittanceColor) == AI_SUCCESS) {
          material.transmittance = { transmittanceColor.r, transmittanceColor.g, transmittanceColor.b };
        }
        float shininess{ 1.f };
        if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
          material.shininess = (0.0f <= shininess) ? shininess : 1.f;
        }

        aiColor3D emissionColor;
        if (mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissionColor) == AI_SUCCESS) {
          material.emission = { emissionColor.r, emissionColor.g, emissionColor.b };
        }
        float ior{ 1.f };
        if (mat->Get(AI_MATKEY_REFRACTI, ior) == AI_SUCCESS) {
          material.ior = ior;
        }
        float dissolve{ 1.f };
        if (mat->Get(AI_MATKEY_OPACITY, dissolve) == AI_SUCCESS) {
          material.dissolve = dissolve;
        }

        if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) {
          aiString ambientTexturePath;
          if (mat->GetTexture(aiTextureType_AMBIENT, 0, &ambientTexturePath) == AI_SUCCESS) {
              material.name_texture_ambient = AssimpLoader::cleanName(ambientTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
          aiString diffuseTexturePath;
          if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath) == AI_SUCCESS) {
              material.name_texture_diffuse = AssimpLoader::cleanName(diffuseTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
          aiString specularTexturePath;
          if (mat->GetTexture(aiTextureType_SPECULAR, 0, &specularTexturePath) == AI_SUCCESS) {
              material.name_texture_specular = AssimpLoader::cleanName(specularTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_SHININESS) > 0) {
          aiString specularHighLightTexturePath;
          if (mat->GetTexture(aiTextureType_SHININESS, 0, &specularHighLightTexturePath) == AI_SUCCESS) {
              material.name_texture_specular_highlight = AssimpLoader::cleanName(specularHighLightTexturePath.C_Str());
          }
        }

        bool const is_obj {std::filesystem::path(path).extension() == ".obj"};

        if (is_obj) {
          if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {
            aiString bumpTexturePath;
            if (mat->GetTexture(aiTextureType_HEIGHT, 0, &bumpTexturePath) == AI_SUCCESS) {
                material.name_texture_bump = AssimpLoader::cleanName(bumpTexturePath.C_Str());
            }
          }
        } else {
          if (mat->GetTextureCount(aiTextureType_NORMALS) > 0) {
            aiString bumpTexturePath;
            if (mat->GetTexture(aiTextureType_NORMALS, 0, &bumpTexturePath) == AI_SUCCESS) {
                material.name_texture_bump = AssimpLoader::cleanName(bumpTexturePath.C_Str());
            }
          }
        }
        if (mat->GetTextureCount(aiTextureType_OPACITY) > 0) {
          aiString alphaTexturePath;
          if (mat->GetTexture(aiTextureType_OPACITY, 0, &alphaTexturePath) == AI_SUCCESS) {
              material.name_texture_alpha = AssimpLoader::cleanName(alphaTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
          aiString metalRoughnessTexturePath;
          if (mat->GetTexture(aiTextureType_UNKNOWN, 0, &metalRoughnessTexturePath) == AI_SUCCESS) {
              material.name_texture_metal_roughness = AssimpLoader::cleanName(metalRoughnessTexturePath.C_Str());
          }
        }
        //int illumModel;
        //if (mat->Get(AI_MATKEY_ILLUM, illumModel) == AI_SUCCESS) {
        //  material.illum = material.illum;
        //}
        materials.emplace_back(material);
      }
    }

    std::vector<Animation> animations{};
    std::vector<Rotation> rotations{};
    std::vector<Position> positions{};
    std::vector<Scale> scales{};


    animations.reserve(scene->mNumAnimations);
    for (unsigned int i{ 0 }; i < scene->mNumAnimations; i++) {

      aiAnimation const* animation = scene->mAnimations[i];

      animations.emplace_back(i, animation->mName.C_Str(), animation->mDuration);

      //PLP_DEBUG("Animation {}, duration {}", animation->mName.C_Str(), animation->mDuration);
      for (unsigned int j{ 0 }; j < animation->mNumChannels; j++) {
        aiNodeAnim const* node = animation->mChannels[j];

        rotations.reserve(node->mNumRotationKeys);
        for (unsigned int r{ 0 }; r < node->mNumRotationKeys; r++) {
          aiQuatKey const& rotKey = node->mRotationKeys[r];
          //PLP_DEBUG("rot {} x {} y {} z {}", rotKey.mTime, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z);
          //rotations.emplace_back(i, rotKey.mTime, GetGLMQuat(rotKey.mValue));
          rotations.emplace_back(i, rotKey.mTime, GetGLMQuat(rotKey.mValue));
        }

        positions.reserve(node->mNumPositionKeys);
        for (unsigned int p{ 0 }; p < node->mNumPositionKeys; p++) {
          aiVectorKey const& posKey = node->mPositionKeys[p];
          //PLP_DEBUG("pos {} x {} y {} z {}", posKey.mTime, posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
          positions.emplace_back(i, posKey.mTime, GetGLMVec(posKey.mValue));
        }

        scales.reserve(node->mNumScalingKeys);
        for (unsigned int s{ 0 }; s < node->mNumScalingKeys; s++) {
          aiVectorKey const& scaleKey = node->mScalingKeys[s];
          //PLP_DEBUG("scale {} x {} y {} z {}", scaleKey.mTime, scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);
          scales.emplace_back(i, scaleKey.mTime, GetGLMVec(scaleKey.mValue));
        }
      }
    }

    std::vector<PlpMeshData> mesh_data{};
    process(scene->mRootNode, scene, mesh_data, flip_Y);

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
    bool const flip_Y)
  {
    for (unsigned int i{ 0 }; i < node->mNumMeshes; i++) {
      PlpMeshData mesh_data{};

      aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];
      mesh_data.name = mesh->mName.C_Str() + std::to_string(i);
      unsigned int count{ 0 };
      glm::vec3 n{ 0.5f, 0.5f, 1.0f };

      mesh_data.vertices.reserve(mesh->mNumVertices);

      for (unsigned int v{ 0 }; v < mesh->mNumVertices; v++) {

        aiVector3D vertices = mesh->mVertices[v];

        Vertex vertex{};
        vertex.bones_ids.resize(4);
        std::fill(vertex.bones_ids.begin(), vertex.bones_ids.end(), -1);
        vertex.weights.resize(4);
        std::fill(vertex.weights.begin(), vertex.weights.end(), 0.0f);

        vertex.pos = { vertices.x, vertices.y, vertices.z };
        vertex.normal = n;

        if (mesh->HasNormals()) {
          aiVector3D const& normal = mesh->mNormals[v];
          vertex.normal = { normal.x, normal.y, normal.z };
          //if (flip_Y) vertex.normal.y = 1.0f - vertex.normal.y;
        } else {
          PLP_WARN("NO NORMAL");
        }

        if (flip_Y) vertex.pos.y *= -1.0f;

        glm::vec4 tangent(0.f, 0.f, 1.f, 1.f);
        glm::vec4 bitangent(0.f, 0.f, 1.f, 1.f);

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
        }
        vertex.tangent = tangent;
        vertex.bitangent = bitangent;

        if (mesh->mNumUVComponents[0] > 0) {
          aiVector3D texture_coord = mesh->mTextureCoords[0][v];
          vertex.texture_coord = { texture_coord.x, texture_coord.y };
          //if (flip_Y) vertex.texture_coord.y *= -1.0f;
        }

        glm::vec4 color{ 1.0f, 0.0f, 0.0f, 1.0f };

        if (mesh->HasVertexColors(v)) {
          color = glm::vec4(
            mesh->mColors[v]->r,
            mesh->mColors[v]->g,
            mesh->mColors[v]->b,
            mesh->mColors[v]->a);
          PLP_DEBUG("r: {} g: {} b: {} a: {}", color.x, color.y, color.z, color.w);
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

      std::unordered_map<std::string, Bone> bonesMap{};
      unsigned int boneCounter{ 0 };

      if (mesh->HasBones()) {

        for (unsigned int b{ 0 }; b < mesh->mNumBones; b++) {
          aiBone const* bone = mesh->mBones[b];

          int boneID{ -1 };
          std::string const& boneName{ bone->mName.C_Str() };

          if (bonesMap.contains(boneName)) {
            boneID = bonesMap[boneName].id;
          } else {
            Bone boneData{};
            boneData.id = boneCounter;
            boneData.name = boneName;
            boneData.offset_matrix = ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
            bonesMap.emplace(boneName, std::move(boneData));

            boneCounter++;
          }

          std::unordered_map<unsigned int, float> weights{};

          for (unsigned int w{ 0 }; w < bone->mNumWeights; w++) {
            aiVertexWeight aiWeight = bone->mWeights[w];

            auto const id = aiWeight.mVertexId;
            auto const weight = aiWeight.mWeight;

            auto& vtex = mesh_data.vertices.at(id);
            for (auto bW{ 0 }; bW < 4; bW++) {
              if (vtex.weights[bW] < 0) {
                vtex.weights[bW] = weight;
                vtex.bones_ids[bW] = boneID;
              }
            }
          }
        }
      }
      data.emplace_back(mesh_data);
    }

    for (unsigned int i{ 0 }; i < node->mNumChildren; i++) {
      process(node->mChildren[i], scene, data, flip_Y);
    }
  }
}
