#include "rebulkpch.h"
#include "Mesh.h"

namespace Rbk
{
    Mesh::Mesh() : Entity(), Drawable() { }

    void Mesh::Init(
         std::string name,
         std::string path,
         std::vector<std::string> textureNames,
         std::string shader,
         glm::vec3 pos,
         glm::vec3 scale,
         glm::vec3 axisRot,
         float rotAngle,
         bool shouldInverseTextureY)
    {
        //@todo move out of Mesh
        if (!std::filesystem::exists(path)) {
            Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        std::vector<Data> listData = Rbk::TinyObjLoader::LoadData(path, shouldInverseTextureY);
        //end todo

        for (int i = 0; i < listData.size(); i++) {

            uint32_t textureIndex = listData[i].materialId;
            std::string id = name + '_' + textureNames[textureIndex] + '_' + std::to_string(i);

            m_Name = id;
            m_Texture = textureNames[textureIndex];
            m_Shader = shader;

            glm::mat4 view = glm::mat4(1.0f);

            UniformBufferObject ubo;
            ubo.model = glm::mat4(1.0f);
            ubo.model = glm::translate(ubo.model, pos);
            ubo.model = glm::scale(ubo.model, scale);

            if (rotAngle != 0.0f) {
                ubo.model = glm::rotate(ubo.model, glm::radians(rotAngle), axisRot);
            }

            ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
            m_Ubos.emplace_back(ubo);
        }
    }

    Mesh::~Mesh()
    {

    }
}