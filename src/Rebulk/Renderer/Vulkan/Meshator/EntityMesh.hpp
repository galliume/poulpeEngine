#pragma once

#include "Rebulk/Core/IVisitor.hpp"
#include "Rebulk/Manager/EntityManager.hpp"
#include "Rebulk/Manager/ShaderManager.hpp"
#include "Rebulk/Manager/TextureManager.hpp"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.hpp"

namespace Rbk
{
    class EntityMesh : public IVisitor
    {
        struct pc
        {
            uint32_t textureID;
            glm::vec3 cameraPos;
            float ambiantLight;
            float fogDensity;
            glm::vec3 fogColor;
            glm::vec3 lightPos;
        };

    public:
        EntityMesh(
            std::shared_ptr<VulkanAdapter> adapter,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<TextureManager> textureManager,
            VkDescriptorPool descriptorPool
        );
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        void CreateBBoxEntity(std::shared_ptr<Mesh>& mesh);

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        VkDescriptorPool m_DescriptorPool;
        std::mutex m_Lock;
    };
}
