#include "rebulkpch.h"
#include "VulkanSplash.h"

namespace Rbk
{
     VulkanSplash::VulkanSplash(
        std::shared_ptr<VulkanAdapter> adapter,
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_DescriptorPool(descriptorPool)
    {

    }

    void VulkanSplash::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

        if (!mesh) return;

        const std::vector<Vertex2D> vertices = {
            {{1.f, -1.f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.f, -1.f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-1.f, 1.f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{1.f, 1.f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        Data data;
        data.m_Texture = "splashscreen";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertex2DBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        data.m_Ubos.emplace_back(ubo);
        data.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        mesh->SetName("splashscreen");
        mesh->SetShaderName("2d");

        std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(uniformBuffer);

        Texture texture = m_Adapter->GetTextureManager()->GetTextures()["splashscreen"];

        std::vector<VkDescriptorImageInfo>imageInfos;
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.GetImageView();
        imageInfo.sampler = texture.GetSampler();

        imageInfos.emplace_back(imageInfo);

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = imageInfos.size();
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayout desriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(
            bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        m_Adapter->GetDescriptorSetLayouts()->emplace_back(desriptorSetLayout);

        for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
            VkDescriptorSet descriptorSet = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets(mesh->m_UniformBuffers, descriptorSet, imageInfos);
            mesh->m_DescriptorSets.emplace_back(descriptorSet);
        }

        VulkanSplash::pc pc;
        pc.textureID = VulkanAdapter::s_Crosshair;

        mesh->ApplyPushConstants = [&pc](VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
            pc.textureID = 0;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        };
        mesh->SetHasPushConstants();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(pc);
        vkPc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);

        std::vector<VkDescriptorSetLayout>dSetLayout = { desriptorSetLayout };

        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, dSetLayout, vkPcs);

        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders[mesh->GetShaderName()][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders[mesh->GetShaderName()][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        VkVertexInputBindingDescription bDesc2D = Vertex::GetBindingDescription();
        auto desc = Vertex2D::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo2D{};
        vertexInputInfo2D.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo2D.vertexBindingDescriptionCount = 1;
        vertexInputInfo2D.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2D::GetAttributeDescriptions().size());
        vertexInputInfo2D.pVertexBindingDescriptions = &bDesc2D;
        vertexInputInfo2D.pVertexAttributeDescriptions = desc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
            shadersStageInfos,
            vertexInputInfo2D,
            VK_CULL_MODE_NONE,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        mesh->GetData()->emplace_back(data);
    }
}
