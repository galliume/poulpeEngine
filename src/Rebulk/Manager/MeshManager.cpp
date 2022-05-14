#include "rebulkpch.h"
#include "MeshManager.h"

namespace Rbk
{
    MeshManager::MeshManager(VulkanRenderer* renderer) : m_Renderer(renderer)
    {

    }

    std::shared_ptr<Mesh> MeshManager::Load(const char* path, bool shouldInverseTextureY)
    {
        if (!std::filesystem::exists(path)) {
            Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        return Rbk::TinyObjLoader::LoadMesh(path, shouldInverseTextureY);
    }

    void MeshManager::AddSkyboxMesh(const char* name, glm::vec3 pos, glm::vec3 scale, bool shouldInverseTextureY)
    {
        m_SkyboxMesh = Load("assets/mesh/cube/cube.obj", shouldInverseTextureY);
        m_SkyboxMesh.get()->texture = "skybox";

        glm::mat4 view = glm::mat4(1.0f);

        UniformBufferObject ubo;
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::translate(ubo.model, pos);
        ubo.model = glm::scale(ubo.model, scale);
        ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(60.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 256.0f);
        ubo.proj[1][1] *= -1;

        m_SkyboxMesh.get()->ubos.emplace_back(ubo);

        Rbk::Log::GetLogger()->debug("Added skybox mesh to the world {}", name);
    }

    void MeshManager::AddWorldMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, glm::vec3 scale, bool shouldInverseTextureY)
    {
        std::shared_ptr<Mesh> mesh = nullptr;

        if (0 == m_WorldMeshesLoaded.count(name)) {
            mesh = Load(path, shouldInverseTextureY);
            mesh.get()->name = name;
            mesh.get()->texture = textureName;
        } else {
            mesh = m_WorldMeshes[m_WorldMeshesLoaded[name][1]];
        }

        glm::mat4 view = glm::mat4(1.0f);

        UniformBufferObject ubo;
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::translate(ubo.model, pos);
        ubo.model = glm::scale(ubo.model, scale);
        ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(60.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
        ubo.proj[1][1] *= -1;
        
        mesh.get()->ubos.emplace_back(ubo);

        if (0 != m_WorldMeshesLoaded.count(name)) {
            m_WorldMeshesLoaded[name][0] += 1;
            m_WorldMeshes[m_WorldMeshesLoaded[name][1]] = mesh;
            return;
        } 
        
        uint32_t index = m_WorldMeshes.size();
        m_WorldMeshesLoaded.insert({ name, { 1, index }});	
        m_WorldMeshes.emplace_back(mesh);

        Rbk::Log::GetLogger()->debug("Added mesh to the world {} from {}", name, path);
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