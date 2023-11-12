#pragma once

#include "IEntity.hpp"

namespace Poulpe
{
    class Grid : public IEntity, public IVisitor
    {
    struct pc
    {
        glm::vec4 point;
        glm::mat4 view;
    };
    public:
        Grid(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager,
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
        VulkanAdapter* m_Adapter;
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        VkDescriptorPool m_DescriptorPool;

    };
}
