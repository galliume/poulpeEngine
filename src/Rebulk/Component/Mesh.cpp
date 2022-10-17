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
        glm::vec3 rotation,
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
        std::vector<BBox> bboxs;

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

            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            ubo.view = glm::mat4(1.0f);
            data.m_Ubos.emplace_back(ubo);

            m_Data.emplace_back(data);

            float xMax = listData[i].vertices.at(0).pos.x;
            float yMax = listData[i].vertices.at(0).pos.y;
            float zMax = listData[i].vertices.at(0).pos.z;

            float xMin = xMax;
            float yMin = xMin;
            float zMin = xMin;

            for (int j = 0; j < listData[i].vertices.size(); j++) {

                glm::vec3 vertex = glm::vec4(listData[i].vertices.at(j).pos, 1.0f);

                float x = vertex.x;
                float y = vertex.y;
                float z = vertex.z;

                if (x > xMax) xMax = x;
                if (x < xMin) xMin = x;
                if (y < yMin) yMin = y;
                if (y > yMax) yMax = y;
                if (z > zMax) zMax = z;
                if (z < zMin) zMin = z;
            }

            glm::vec3 center = glm::vec3((xMin + xMax) / 2, (yMin + yMax) / 2, (zMin + zMax) / 2);
            glm::vec3 size = glm::vec3(xMax - xMin, yMax - yMin, zMax - zMin);

            BBox box;
            box.center = center;
            box.size = size;
            box.scale = scale;
            box.rotation = rotation;
            bboxs.emplace_back(box);

        }
        SetBBox(bboxs);
    }

    void Mesh::AddUbos(const std::vector<UniformBufferObject>& ubos)
    {
        for (auto& data : m_Data) {
            data.m_Ubos.insert(data.m_Ubos.end(), ubos.begin(), ubos.end());
        }
    }

    Mesh::~Mesh() { }
}
