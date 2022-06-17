#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Component/Mesh.h"
#include "Rebulk/Component/Entity.h"

namespace Rbk
{
    class EntityManager
    {
    public:

        explicit EntityManager(std::shared_ptr<VulkanRenderer> renderer);
        void AddEntity(std::shared_ptr<Entity> entity);
        inline std::map<std::string, std::array<uint32_t, 2>> GetMeshesLoaded() { return m_MeshesLoaded; };
        uint32_t GetVerticesCount();
        uint32_t GetIndicesCount();
        uint32_t GetInstancedCount();
        inline uint32_t GetTotalEntities() { return m_Entities.size(); };
        inline std::shared_ptr<Mesh> GetSkyboxMesh() { return m_SkyboxMesh; };
        inline void SetSkyboxMesh(std::shared_ptr<Mesh> skyboxMesh) { m_SkyboxMesh = skyboxMesh; };
        inline std::vector<std::shared_ptr<Entity>>* GetEntities() { return &m_Entities; };

    private:
        std::vector<std::shared_ptr<Entity>> m_Entities;
        std::shared_ptr<Mesh> m_SkyboxMesh = nullptr;
        std::map<std::string, std::array<uint32_t, 2>> m_MeshesLoaded;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
    };
}