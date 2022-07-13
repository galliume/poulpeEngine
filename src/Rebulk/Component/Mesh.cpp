#include "rebulkpch.h"
#include "Mesh.h"

namespace Rbk
{
    Mesh::Mesh() : Entity() { }

    void Mesh::Init(const std::string& name,
        const std::string& path,
        const std::vector<std::string>& textureNames,
        const std::string& shader,
        const glm::vec3& pos,
        const glm::vec3& scale,
        glm::vec3 axisRot,
        float rotAngle,
        bool shouldInverseTextureY)
    {
        //@todo move out of Mesh
        if (!std::filesystem::exists(path)) {
            Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        std::vector<TinyObjData> listData = Rbk::TinyObjLoader::LoadData(path, shouldInverseTextureY);
        //end todo

        SetName(name);
        m_ShaderName = shader;

        for (size_t i = 0; i < listData.size(); i++) {

            Data data;

            data.m_Name = name + '_' + textureNames[listData[i].materialId] + '_' + std::to_string(i);
            data.m_Texture = textureNames[listData[i].materialId];
            data.m_Vertices = listData[i].vertices;
            data.m_Indices = listData[i].indices;

            glm::mat4 view = glm::mat4(1.0f);

            UniformBufferObject ubo;
            ubo.model = glm::mat4(1.0f);
            ubo.model = glm::translate(ubo.model, pos);
            ubo.model = glm::scale(ubo.model, scale);

            if (rotAngle != 0.0f) {
                ubo.model = glm::rotate(ubo.model, glm::radians(rotAngle), axisRot);
            }

            ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
            data.m_Ubos.emplace_back(ubo);

            m_Data.emplace_back(data);
        }
    }

    void Mesh::AddUbos(const std::vector<UniformBufferObject>& ubos)
    {
        for (auto& data : m_Data) {
            data.m_Ubos.insert(data.m_Ubos.end(), ubos.begin(), ubos.end());
        }
    }

    Mesh::~Mesh() { }
}
