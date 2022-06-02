#include "rebulkpch.h"
#include "MeshManager.h"

namespace Rbk
{
    MeshManager::MeshManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {

    }

    std::vector<std::shared_ptr<Mesh>> MeshManager::Load(std::string path, bool shouldInverseTextureY)
    {
        if (!std::filesystem::exists(path)) {
            Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        return Rbk::TinyObjLoader::LoadMesh(path, shouldInverseTextureY);
    }

    void MeshManager::AddWorldMesh(std::string name, std::string path, std::vector<std::string> textureNames, glm::vec3 pos, glm::vec3 scale, glm::vec3 axisRot, float rotAngle, bool shouldInverseTextureY)
    {
        std::vector<std::shared_ptr<Mesh>> meshes = Load(path, shouldInverseTextureY);

        for (uint32_t i = 0; i < meshes.size(); i++) {

            std::shared_ptr<Mesh> mesh = meshes[i];
            uint32_t textureIndex = mesh.get()->materialId;
            std::string id = name + '_' + textureNames[textureIndex] + '_' + std::to_string(i);

            if (0 == m_WorldMeshesLoaded.count(id.c_str())) {
                mesh.get()->name = id;
                mesh.get()->texture = textureNames[textureIndex];
            } else {
                mesh = m_WorldMeshes[m_WorldMeshesLoaded[id][1]];
            }

            glm::mat4 view = glm::mat4(1.0f);

            UniformBufferObject ubo;
            ubo.model = glm::mat4(1.0f);
            ubo.model = glm::translate(ubo.model, pos);
            ubo.model = glm::scale(ubo.model, scale);
            
            if (rotAngle != 0.0f) {
                ubo.model = glm::rotate(ubo.model, glm::radians(rotAngle), axisRot);
            }

            ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
            mesh.get()->ubos.emplace_back(ubo);

            if (0 != m_WorldMeshesLoaded.count(id.c_str())) {
                m_WorldMeshesLoaded[id][0] += 1;
                //m_WorldMeshes[m_WorldMeshesLoaded[name][1]] = mesh;
            } else {

                uint32_t index = m_WorldMeshes.size();
                m_WorldMeshesLoaded.insert({ id, { 1, index } });
                m_WorldMeshes.emplace_back(mesh);

                Rbk::Log::GetLogger()->trace("Added mesh to the world {} from {}", id, path);
            }
        }
    }

    uint32_t MeshManager::GetWorldVerticesCount()
    {
        uint32_t total = 0;

        for (std::shared_ptr<Mesh> mesh : m_WorldMeshes) {
            total += mesh.get()->vertices.size();
        }

        return total;
    }

    uint32_t MeshManager::GetWorldIndicesCount()
    {
        uint32_t total = 0;

        for (std::shared_ptr<Mesh>mesh : m_WorldMeshes) {
            total += mesh.get()->indices.size();
        }

        return total;
    }

    uint32_t MeshManager::GetWorldInstancedCount()
    {
        uint32_t total = 0;

        for (std::shared_ptr<Mesh> mesh : m_WorldMeshes) {
            total += mesh.get()->ubos.size();
        }

        return total;
    }
}