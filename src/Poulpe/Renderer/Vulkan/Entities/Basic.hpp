#pragma once

#include "IMesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

namespace Poulpe
{
    class Basic : public IVisitor, public IMesh
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
        Basic(VulkanAdapter* adapter,  ShaderManager* shaderManager, TextureManager* textureManager,
              VkDescriptorPool descriptorPool);

        void visit(float const deltaTime, Mesh* mesh) override;

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
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
