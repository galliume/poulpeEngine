#include "AssimpLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Poulpe
{
  void AssimpLoader::loadData(
        std::string const & path,
        bool const shouldInverseTextureY,
        std::function<void(
          PlpMeshData const& _data,
          std::vector<material_t> const& materials,
          bool const exists)> callback)
  {
    Assimp::Importer importer;
  
    const aiScene* scene = importer.ReadFile(path,
      aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals 
    );

    if (nullptr == scene) {
      PLP_ERROR("Error while importing file {}: {}", path, importer.GetErrorString());
      return;
    }

    std::vector<material_t> materials{};

    if (scene->HasMaterials()) {
      for (auto i{ 0 }; i < scene->mNumMaterials; ++i) {
       
        auto const& mat = scene->mMaterials[i];

        material_t material{};

        material.name = mat->GetName().C_Str();

        aiColor3D ambientColor(0.f, 0.f, 0.f);
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
          material.ambient = { ambientColor.r, ambientColor.g, ambientColor.b };

        }
        aiColor3D diffuseColor(0.f, 0.f, 0.f);
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
          material.diffuse = { diffuseColor.r, diffuseColor.g, diffuseColor.b };

        }
        aiColor3D specularColor(0.f, 0.f, 0.f);
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
          material.specular = { specularColor.r, specularColor.g, specularColor.b };

        }
        aiColor3D transmittanceColor(0.f, 0.f, 0.f);
        if (mat->Get(AI_MATKEY_COLOR_TRANSPARENT, transmittanceColor) == AI_SUCCESS) {
          material.transmittance = { transmittanceColor.r, transmittanceColor.g, transmittanceColor.b };

        }
        float shininess;
        if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
          material.shininess = shininess;
        }
        aiColor3D emissionColor;
        if (mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissionColor) == AI_SUCCESS) {
          material.emission = { emissionColor.r, emissionColor.g, emissionColor.b };
        }
        float ior;
        if (mat->Get(AI_MATKEY_REFRACTI, ior) == AI_SUCCESS) {
          material.ior = ior;
        }
        float dissolve;
        if (mat->Get(AI_MATKEY_OPACITY, dissolve) == AI_SUCCESS) {
          material.dissolve = dissolve;
        }

        if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) {
          aiString ambientTexturePath;
          if (mat->GetTexture(aiTextureType_AMBIENT, 0, &ambientTexturePath) == AI_SUCCESS) {
              material.ambientTexname = AssimpLoader::cleanName(ambientTexturePath.C_Str());
          }
        } 
        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
          aiString diffuseTexturePath;
          if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath) == AI_SUCCESS) {
              material.diffuseTexname = AssimpLoader::cleanName(diffuseTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
          aiString specularTexturePath;
          if (mat->GetTexture(aiTextureType_SPECULAR, 0, &specularTexturePath) == AI_SUCCESS) {
              material.specularTexname = AssimpLoader::cleanName(specularTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_SHININESS) > 0) {
          aiString specularHighLightTexturePath;
          if (mat->GetTexture(aiTextureType_SHININESS, 0, &specularHighLightTexturePath) == AI_SUCCESS) {
              material.specularHighlightTexname = AssimpLoader::cleanName(specularHighLightTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_HEIGHT) > 0) {
          aiString bumpTexturePath;
          if (mat->GetTexture(aiTextureType_HEIGHT, 0, &bumpTexturePath) == AI_SUCCESS) {
              material.bumpTexname = AssimpLoader::cleanName(bumpTexturePath.C_Str());
          }
        }
        if (mat->GetTextureCount(aiTextureType_OPACITY) > 0) {
          aiString alphaTexturePath;
          if (mat->GetTexture(aiTextureType_OPACITY, 0, &alphaTexturePath) == AI_SUCCESS) {
              material.alphaTexname = AssimpLoader::cleanName(alphaTexturePath.C_Str());
          }
        }
        //int illumModel;
        //if (mat->Get(AI_MATKEY_ILLUM, illumModel) == AI_SUCCESS) {
        //  material.illum = material.illum;
        //}
        materials.emplace_back(material);
      }
    }

    for (unsigned int i{ 0 }; i < scene->mNumMeshes; i++) {
      PlpMeshData meshData{};

      aiMesh const* mesh = scene->mMeshes[i];
      meshData.name = mesh->mName.C_Str() + std::to_string(i);
      unsigned int count{ 0 };

      for (unsigned int f{ 0 }; f < mesh->mNumFaces; f++) {
        aiFace const* face = &mesh->mFaces[f];

        for (unsigned int j{ 0 }; j < face->mNumIndices; j++) {

          aiVector3D vertices = mesh->mVertices[face->mIndices[j]];
          Vertex vertex{};
          vertex.fidtidBB = glm::vec4(static_cast<float>(j), mesh->mMaterialIndex, 0.0f, 0.0f);
          vertex.pos = { vertices.x, vertices.y, vertices.z };

          if (mesh->HasNormals()) {
              aiVector3D const& normal = mesh->mNormals[face->mIndices[j]];
              vertex.normal = { normal.x, normal.y, normal.z };
          } else {
            vertex.normal = { 1.0f, 1.0f, 1.0f };
          }

          if (mesh->mNumUVComponents[0] > 0) {
            aiVector3D texCoord = mesh->mTextureCoords[0][face->mIndices[j]];
            vertex.texCoord = { texCoord.x, texCoord.y };
            if (shouldInverseTextureY) vertex.texCoord.y *= -1.0f;
          }

          vertex.color = { 1.0f, 1.0f, 1.0f };

          meshData.vertices.push_back(std::move(vertex));
          meshData.indices.push_back(count);
          count += 1;
        }
        meshData.materialId = mesh->mMaterialIndex;
        meshData.materialsID = { mesh->mMaterialIndex };
        meshData.facesMaterialId.emplace_back(mesh->mMaterialIndex);
      }
      callback(meshData, materials, false);
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

    return cleaned;
  };
}
