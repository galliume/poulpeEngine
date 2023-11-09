#pragma once

#include "IEntity.hpp"

namespace Poulpe
{
    class Basic : public IEntity, public IVisitor
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
        Basic(
            std::shared_ptr<VulkanAdapter> adapter,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<TextureManager> textureManager,
            VkDescriptorPool descriptorPool
        );
        void visit(std::shared_ptr<Entity> entity) override;
        VkDescriptorSetLayout createDescriptorSetLayout() override;
        std::vector<VkDescriptorSet> createDescriptorSet(std::shared_ptr<Mesh> mesh) override;
        VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) override;
        std::vector<VkPipelineShaderStageCreateInfo> getShaders(std::string const & name) override;
        VkPipelineVertexInputStateCreateInfo getVertexBindingDesc(
          VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc) override;
        void setPushConstants(std::shared_ptr<Mesh> mesh) override;

    private:
        void createBBoxEntity(std::shared_ptr<Mesh> & mesh);

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        VkDescriptorPool m_DescriptorPool;
        std::mutex m_Lock;
    };
}
