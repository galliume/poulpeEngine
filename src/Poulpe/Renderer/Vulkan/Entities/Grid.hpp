#pragma once

#include "IMesh.hpp"

#include "Poulpe/Component/Component.hpp"

namespace Poulpe
{
    class Grid : public Component, public IMesh, public IVisitor
    {
    struct pc
    {
        glm::vec4 point;
        glm::mat4 view;
    };
    public:
        Grid(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager);

        void visit(Mesh* meshy) override;

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
