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
            std::shared_ptr<TextureManager> textureManager,
            VkDescriptorPool descriptorPool
        );
        void Visit(std::shared_ptr<Entity> entity) override;
        VkDescriptorSetLayout CreateDescriptorSetLayout(std::shared_ptr<Mesh> mesh);
        VkDescriptorSet CreateDescriptorSet(std::shared_ptr<Mesh> mesh, VkDescriptorSetLayout descriptorSetLayout);
        VkPipelineLayout CreatePipelineLayout(std::shared_ptr<Mesh> mesh, VkDescriptorSetLayout descriptorSetLayout);
        std::vector<VkPipelineShaderStageCreateInfo> GetShaders();
        VkPipelineVertexInputStateCreateInfo GetVertexBindingDesc(
          VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc);
        void SetPushConstants(std::shared_ptr<Mesh> mesh);

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
