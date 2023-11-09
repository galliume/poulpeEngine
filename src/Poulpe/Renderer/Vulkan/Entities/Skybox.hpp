#pragma once

#include "Poulpe/Core/IVisitor.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class Skybox : public IVisitor
    {
    public:
        Skybox(
            std::shared_ptr<VulkanAdapter> adapter,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<TextureManager> textureManager
        );
        void visit(std::shared_ptr<Entity> entity) override;
        VkDescriptorSetLayout createDescriptorSetLayout();
        VkDescriptorSet createDescriptorSet(std::shared_ptr<Mesh> mesh, VkDescriptorSetLayout descriptorSetLayout);
        VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
        std::vector<VkPipelineShaderStageCreateInfo> getShaders();
        VkPipelineVertexInputStateCreateInfo getVertexBindingDesc(
          VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc);
        void setPushConstants(std::shared_ptr<Mesh> mesh);

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        //VkDescriptorPool m_DescriptorPool;
    };
}
