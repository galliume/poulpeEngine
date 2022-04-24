#include "rebulkpch.h"
#include "MeshManager.h"

namespace Rbk
{
    MeshManager::MeshManager(VulkanRenderer* renderer) : m_Renderer(renderer)
    {

    }

    Mesh MeshManager::Load(const char* path, bool shouldInverseTextureY)
    {
        if (!std::filesystem::exists(path)) {
            Rbk::Log::GetLogger()->critical("mesh file {} does not exits.", path);
            throw std::runtime_error("error loading a mesh file.");
        }

        return Rbk::TinyObjLoader::LoadMesh(path, shouldInverseTextureY);
    }

    void MeshManager::AddSkyboxMesh(const char* name, glm::vec3 pos, glm::vec3 scale, bool shouldInverseTextureY)
    {
        float skyboxVertices[][3] = { 
            { -1.0f, -1.0f,  1.0f },
            {  1.0f, -1.0f,  1.0f },
            {  1.0f,  1.0f,  1.0f },
            { -1.0f,  1.0f,  1.0f },

            {  1.0f,  1.0f,  1.0f },
            {  1.0f,  1.0f, -1.0f },
            {  1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f,  1.0f },

            { -1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f },
            { -1.0f,  1.0f, -1.0f },

            { -1.0f, -1.0f, -1.0f },
            { -1.0f, -1.0f,  1.0f },
            { -1.0f,  1.0f,  1.0f },
            { -1.0f,  1.0f, -1.0f },

            {  1.0f,  1.0f,  1.0f },
            { -1.0f,  1.0f,  1.0f },
            { -1.0f,  1.0f, -1.0f },
            {  1.0f,  1.0f, -1.0f },

            { -1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f, -1.0f },
            {  1.0f, -1.0f,  1.0f },
            { -1.0f, -1.0f,  1.0f },
        };

        std::vector<uint32_t> indices = {
            0,1,2, 0,2,3, 4,5,6,  4,6,7, 8,9,10, 8,10,11, 12,13,14, 12,14,15, 16,17,18, 16,18,19, 20,21,22, 20,22,23
        };

        Mesh mesh;

        for (auto vert : skyboxVertices) {
            Vertex vertex;
            vertex.pos = glm::vec3(vert[0], vert[1], vert[2]);
            vertex.texCoord = glm::vec3(vert[0], vert[1], vert[2]);
            vertex.color = glm::vec3(0.5f, 0.5f, 0.5);
            mesh.vertices.emplace_back(vertex);
        }

        for (uint32_t i : indices) {
            mesh.indices.emplace_back(i);
        }

        mesh.name = name;
        
        glm::mat4 view = glm::mat4(1.0f);

        CubeUniformBufferObject ubo;
        ubo.model = glm::mat4(1.0f);
        ubo.view = glm::mat4(glm::mat3(glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        )));

        float s = (float)m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height;

        ubo.proj = glm::perspective(glm::radians(60.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
        ubo.proj[1][1] *= -1;

        float offset = 1.0f;
        float center = (6 * offset) / 2.0f - (offset * 0.5f);
        for (uint32_t i = 0; i < 6; i++) {
            ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(i * offset - center, 0.0f, 0.0f));
            ubo.model = glm::scale(ubo.model, glm::vec3(1.0f));
            ubo.index = i;
            mesh.ubos.emplace_back(ubo);
        }

        m_SkyboxMesh = mesh;

        Rbk::Log::GetLogger()->debug("Added skybox mesh to the world {}", name);
    }

    void MeshManager::AddWorldMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, glm::vec3 scale, bool shouldInverseTextureY)
    {		
        Mesh mesh;

        if (0 == m_WorldMeshesLoaded.count(name)) {
            mesh = Load(path, shouldInverseTextureY);
            mesh.name = name;
            mesh.texture = textureName;
        } else {
            mesh = m_WorldMeshes[m_WorldMeshesLoaded[name][1]];
        }

        glm::mat4 view = glm::mat4(1.0f);

        UniformBufferObject ubo;
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::translate(ubo.model, pos);
        ubo.model = glm::scale(ubo.model, scale);
        ubo.view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
        
        float s = (float) m_Renderer->GetSwapChainExtent().width /  (float) m_Renderer->GetSwapChainExtent().height;
        
        //ubo.proj = Rbk::Camera::FrustumProj(glm::radians(60.0f), s, 0.1f, 100.0f);
        ubo.proj = glm::perspective(glm::radians(60.0f), m_Renderer->GetSwapChainExtent().width / (float)m_Renderer->GetSwapChainExtent().height, 0.1f, 100.0f);
        ubo.proj[1][1] *= -1;
        
        mesh.ubos.emplace_back(ubo);

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

        for (Mesh mesh : m_WorldMeshes) {
            total += mesh.vertices.size();
        }

        return total;
    }

    uint32_t MeshManager::GetWorldIndicesCount()
    {
        uint32_t total = 0;

        for (Mesh mesh : m_WorldMeshes) {
            total += mesh.indices.size();
        }

        return total;
    }

    uint32_t MeshManager::GetWorldInstancedCount()
    {
        uint32_t total = 0;

        for (Mesh mesh : m_WorldMeshes) {
            total += mesh.ubos.size();
        }

        return total;
    }
}