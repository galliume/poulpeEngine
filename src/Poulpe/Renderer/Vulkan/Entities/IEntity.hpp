#pragma once

#include "Poulpe/Core/IVisitor.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class IEntity
    {
    public:
        virtual VkDescriptorSetLayout createDescriptorSetLayout() = 0;
        virtual std::vector<VkDescriptorSet> createDescriptorSet(std::shared_ptr<Mesh> mesh) = 0;
        virtual VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) = 0;
        virtual std::vector<VkPipelineShaderStageCreateInfo> getShaders(std::string const & name) = 0;
        virtual VkPipelineVertexInputStateCreateInfo getVertexBindingDesc(
            VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc) = 0;
        virtual void setPushConstants(std::shared_ptr<Mesh> mesh) = 0;
    };
}