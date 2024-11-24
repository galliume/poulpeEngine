#include "AssimpLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
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
        bool const inverse_texture_y,
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
  
    const aiScene* scene = importer.ReadFile(path,
      aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace
    );

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

        material.name = mat->GetName().C_Str();

        aiColor3D ambientColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
          material.ambient = { ambientColor.r, ambientColor.g, ambientColor.b };
          if (material.ambient.r == 0) material.ambient = glm::vec3{ 1.f };
        }
        aiColor3D diffuseColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
          material.diffuse = { diffuseColor.r, diffuseColor.g, diffuseColor.b };
          if (material.diffuse.r == 0) material.diffuse = glm::vec3{ 1.f };
        }
        aiColor3D specularColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
          material.specular = { specularColor.r, specularColor.g, specularColor.b };
          if (material.specular.r == 0) material.specular = glm::vec3{ 1.f };
        }
        aiColor3D transmittanceColor(1.f);
        if (mat->Get(AI_MATKEY_COLOR_TRANSPARENT, transmittanceColor) == AI_SUCCESS) {
          material.transmittance = { transmittanceColor.r, transmittanceColor.g, transmittanceColor.b };
          if (material.transmittance.r == 0) material.transmittance = glm::vec3{ 1.f };
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
        if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {
          aiString bumpTexturePath;
          if (mat->GetTexture(aiTextureType_HEIGHT, 0, &bumpTexturePath) == AI_SUCCESS) {
              material.name_texture_bump = AssimpLoader::cleanName(bumpTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_OPACITY) > 0) {
          aiString alphaTexturePath;
          if (mat->GetTexture(aiTextureType_OPACITY, 0, &alphaTexturePath) == AI_SUCCESS) {
              material.name_texture_alpha = AssimpLoader::cleanName(alphaTexturePath.C_Str());
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

    for (unsigned int i{ 0 }; i < scene->mNumMeshes; i++) {
      PlpMeshData meshData{};
     
      aiMesh const* mesh = scene->mMeshes[i];
      meshData.name = mesh->mName.C_Str() + std::to_string(i);
      unsigned int count{ 0 };

      for (unsigned int f{ 0 }; f < mesh->mNumFaces; f++) {
        aiFace const* face = &mesh->mFaces[f];

        meshData.vertices.reserve(face->mNumIndices);
        meshData.indices.reserve(face->mNumIndices);
        meshData.face_material_ID.reserve(face->mNumIndices);

        for (unsigned int j{ 0 }; j < face->mNumIndices; j++) {

          aiVector3D vertices = mesh->mVertices[face->mIndices[j]];
          Vertex vertex{};
          vertex.bones_ids.resize(4);
          std::fill(vertex.bones_ids.begin(), vertex.bones_ids.end(), -1);
          vertex.weights.resize(4);
          std::fill(vertex.weights.begin(), vertex.weights.end(), 0.0f);

          vertex.fidtidBB = glm::vec4(static_cast<float>(j), static_cast<float>(mesh->mMaterialIndex), 0.0f, 0.0f);
          vertex.pos = { vertices.x, vertices.y, vertices.z };

          glm::vec4 tangent(1.f);

          if (nullptr != mesh->mTangents) {
            for (unsigned int t{ 0 }; t < mesh->mNumVertices; ++t) {
              tangent.x += mesh->mTangents[t].x;
              tangent.y += mesh->mTangents[t].y;
              tangent.z += mesh->mTangents[t].z;
            }
            tangent.x /= mesh->mNumVertices;
            tangent.y /= mesh->mNumVertices;
            tangent.z /= mesh->mNumVertices;
          }
          vertex.tangent = tangent;
          

          if (mesh->HasNormals()) {
              aiVector3D const& normal = mesh->mNormals[face->mIndices[j]];
              vertex.normal = { normal.x, normal.y, normal.z };
          } else {
            vertex.normal = { 1.0f, 1.0f, 1.0f };
          }

          if (mesh->mNumUVComponents[0] > 0) {
            aiVector3D texCoord = mesh->mTextureCoords[0][face->mIndices[j]];
            vertex.texCoord = { texCoord.x, texCoord.y };
            if (!inverse_texture_y) vertex.texCoord.y *= -1.0f;
          }

          vertex.color = { 1.0f, 1.0f, 1.0f };

          meshData.vertices.emplace_back(std::move(vertex));
          meshData.indices.push_back(count);
          count += 1;
        }
        meshData.material_ID = mesh->mMaterialIndex;
        meshData.materials_ID = { mesh->mMaterialIndex };
        meshData.face_material_ID.emplace_back(mesh->mMaterialIndex);
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

            auto& vtex = meshData.vertices.at(id);
            for (auto bW{ 0 }; bW < 4; bW++) {
              if (vtex.weights[bW] < 0) {
                vtex.weights[bW] = weight;
                vtex.bones_ids[bW] = boneID;
              }
            }
          }
        }
      }
      callback(std::move(meshData), materials, false, animations, positions, rotations, scales);
      count = meshData.indices.size();
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
  };
}
