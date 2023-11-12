#include "Mesh.hpp"

#include <stdexcept>

namespace Poulpe
{
    Mesh::Mesh() : Entity() { }

    std::vector<Mesh*> Mesh::init(std::string const  & name, std::string const & path,
        std::vector<std::string> const & textureNames, std::string const & shader,
        glm::vec3 const & pos, glm::vec3 const & scale, glm::vec3 rotation,
        bool shouldInverseTextureY)
    {
        //@todo move out of Mesh
        if (!std::filesystem::exists(path)) {
            PLP_FATAL("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        std::vector<TinyObjData> listData = Poulpe::TinyObjLoader::loadData(path, shouldInverseTextureY);
        //end todo

        std::vector<Mesh*> meshes;

        for (size_t i = 0; i < listData.size(); i++) {

            Mesh* mesh = new Mesh();
            mesh->setName(name + '_' + std::to_string(i));
            mesh->setShaderName(shader);

            std::vector<Poulpe::Mesh::BBox> bboxs{};

            Data data{};
            data.m_Name = name + '_' + textureNames[listData[i].materialId];
            data.m_Texture = textureNames[listData[i].materialId];
            data.m_Vertices = listData[i].vertices;
            data.m_Indices = listData[i].indices;

            UniformBufferObject ubo{};
            ubo.model = glm::mat4(1.0f);
            ubo.model = glm::translate(ubo.model, pos);
            ubo.model = glm::scale(ubo.model, scale);

            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            ubo.model = glm::rotate(ubo.model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            //ubo.view = glm::mat4(1.0f);
            data.m_Ubos.emplace_back(ubo);

            float xMax = data.m_Vertices.at(0).pos.x;
            float yMax = data.m_Vertices.at(0).pos.y;
            float zMax = data.m_Vertices.at(0).pos.z;

            float xMin = xMax;
            float yMin = yMax;
            float zMin = zMax;

            for (size_t j = 0; j < data.m_Vertices.size(); j++) {

                glm::vec3 vertex = glm::vec4(data.m_Vertices.at(j).pos, 1.0f);

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
            glm::vec3 size = glm::vec3((xMax - xMin) / 2, (yMax - yMin) / 2, (zMax - zMin) / 2);

            std::shared_ptr<Poulpe::Mesh::BBox> box = std::make_shared<Poulpe::Mesh::BBox>();
            box->position = data.m_Ubos.at(0).model;
            box->center = center;
            box->size = size;
            box->mesh = std::make_unique<Mesh>();
            box->maxX = xMax;
            box->minX = xMin;
            box->maxY = yMax;
            box->minY = yMin;
            box->maxZ = zMax;
            box->minZ = zMin;

            mesh->setData(data);
            mesh->addBBox(box);

            meshes.emplace_back(std::move(mesh));
        }

        return meshes;
    }

    void Mesh::addUbos(const std::vector<UniformBufferObject>& ubos)
    {
        m_Data.m_Ubos.insert(m_Data.m_Ubos.end(), ubos.begin(), ubos.end());
    }
}
