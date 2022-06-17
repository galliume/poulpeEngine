#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "Rebulk/Core/TinyObjLoader.h"
#include "Rebulk/Component/Camera.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class MeshManager
    {
    public:

        explicit MeshManager(std::shared_ptr<VulkanRenderer> renderer);
        void AddMesh(std::string name, std::string path, std::vector<std::string> textureNames, std::string shader, glm::vec3 pos, glm::vec3 scale, glm::vec3 axisRot = glm::vec3(1.0f), float rotAngle = 0.0f, bool shouldInverseTextureY = true);
        inline std::vector<std::shared_ptr<Mesh>>* GetWorldMeshes() { return &m_WorldMeshes; };
        inline std::map<std::string, std::array<uint32_t, 2>> GetWoldMeshesLoaded() { return m_WorldMeshesLoaded; };
        uint32_t GetWorldVerticesCount();
        uint32_t GetWorldIndicesCount();
        uint32_t GetWorldInstancedCount();
        inline uint32_t GetWorldTotalMesh() { return m_WorldMeshes.size(); };
        inline std::shared_ptr<Mesh> GetSkyboxMesh() { return m_SkyboxMesh; };
        inline void SetSkyboxMesh(std::shared_ptr<Mesh> skyboxMesh) { m_SkyboxMesh = skyboxMesh; };
        std::vector<std::shared_ptr<Mesh>> Load(std::string path, bool shouldInverseTextureY);

    private:
        std::vector<std::shared_ptr<Mesh>> m_WorldMeshes;
        std::shared_ptr<Mesh> m_SkyboxMesh = nullptr;
        std::map<std::string, std::array<uint32_t, 2>> m_WorldMeshesLoaded;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
    };
}