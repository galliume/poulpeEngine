#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Component/Camera.h"

namespace Rbk
{
    class MeshManager
    {
    public:

        MeshManager(VulkanRenderer* renderer);
        void AddWorldMesh(const char* name, const char* path, const char* textureName, glm::vec3 pos, glm::vec3 scale, bool shouldInverseTextureY = true);
        void AddSkyboxMesh(const char* name, glm::vec3 pos, glm::vec3 scale, bool shouldInverseTextureY = true);
        inline std::vector<Mesh>* GetWorldMeshes() { return &m_WorldMeshes; };
        inline std::map<const char*, std::array<uint32_t, 2>> GetWoldMeshesLoaded() { return m_WorldMeshesLoaded; };
        uint32_t GetWorldVerticesCount();
        uint32_t GetWorldIndicesCount();
        uint32_t GetWorldInstancedCount();
        inline uint32_t GetWorldTotalMesh() { return m_WorldMeshes.size(); };
        inline Mesh* GetSkyboxMesh() { return &m_SkyboxMesh; };

    private:
        Mesh Load(const char* path, bool shouldInverseTextureY);

    private:
        std::vector<Mesh> m_WorldMeshes;
        Mesh m_SkyboxMesh;
        std::map<const char*, std::array<uint32_t, 2>> m_WorldMeshesLoaded;
        VulkanRenderer* m_Renderer = nullptr;
    };
}