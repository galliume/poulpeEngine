#pragma once

#include "Poulpe/Core/IVisitor.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class Basic : public IVisitor
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
        void Visit(std::shared_ptr<Entity> entity) override;
        VkDescriptorSetLayout CreateDescriptorSetLayout();
        std::vector<VkDescriptorSet> CreateDescriptorSet(std::shared_ptr<Mesh> mesh, VkDescriptorSetLayout descriptorSetLayout);
        VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
        std::vector<VkPipelineShaderStageCreateInfo> GetShaders(std::string const & shaderName);
        VkPipelineVertexInputStateCreateInfo GetVertexBindingDesc(
          VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc);
        void SetPushConstants(std::shared_ptr<Mesh> mesh);

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
