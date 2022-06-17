#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class EntityManager
    {
    public:

        explicit EntityManager(std::shared_ptr<VulkanRenderer> renderer);
        void AddMesh(std::string name, std::string path, std::vector<std::string> textureNames, std::string shader, glm::vec3 pos, glm::vec3 scale, glm::vec3 axisRot = glm::vec3(1.0f), float rotAngle = 0.0f, bool shouldInverseTextureY = true);
        inline std::vector<std::shared_ptr<Mesh>>* GetMeshes() { return &m_Meshes; };
        inline std::map<std::string, std::array<uint32_t, 2>> GetMeshesLoaded() { return m_MeshesLoaded; };
        uint32_t GetVerticesCount();
        uint32_t GetIndicesCount();
        uint32_t GetInstancedCount();
        inline uint32_t GetTotalMesh() { return m_Meshes.size(); };
        inline std::shared_ptr<Mesh> GetSkyboxMesh() { return m_SkyboxMesh; };
        inline void SetSkyboxMesh(std::shared_ptr<Mesh> skyboxMesh) { m_SkyboxMesh = skyboxMesh; };
        std::vector<std::shared_ptr<Mesh>> Load(std::string path, bool shouldInverseTextureY);

    private:
        std::vector<std::shared_ptr<Mesh>> m_Meshes;
        std::shared_ptr<Mesh> m_SkyboxMesh = nullptr;
        std::map<std::string, std::array<uint32_t, 2>> m_MeshesLoaded;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
    };
}