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
        Basic(VulkanAdapter* adapter, EntityManager* entityManager,
              ShaderManager* shaderManager, TextureManager* textureManager,
              VkDescriptorPool descriptorPool);

        void visit(Entity* entity) override;
        VkDescriptorSetLayout createDescriptorSetLayout() override;
        std::vector<VkDescriptorSet> createDescriptorSet(Mesh* mesh) override;
        VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) override;
        std::vector<VkPipelineShaderStageCreateInfo> getShaders(std::string const & name) override;

        VkPipelineVertexInputStateCreateInfo getVertexBindingDesc(
          VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc) override;

        void setPushConstants(Mesh* mesh) override;

    private:
        void createBBoxEntity(Mesh* mesh);

    private:
        VulkanAdapter* m_Adapter;
        EntityManager* m_EntityManager;
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        VkDescriptorPool m_DescriptorPool;

        std::mutex m_Lock;
    };
}
