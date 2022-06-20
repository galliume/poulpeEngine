#include "rebulkpch.h"
#include "VisitorVulkan.h"

namespace Rbk
{
    struct constants;

    VisitorVulkan::VisitorVulkan(
        std::shared_ptr<VulkanAdapter> adapter, 
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_DescriptorPool(descriptorPool)
    {

    }

    void VisitorVulkan::VisitPrepare(Mesh* mesh)
    {
        Rbk::Log::GetLogger()->debug("Visiting VisitorVulkanInit");

        /*uint32_t totalInstances = static_cast<uint32_t>(entities->size());

        maxUniformBufferRange = m_Renderer->GetDeviceProperties().limits.maxUniformBufferRange;
        uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));
        mesh->m_CameraPos = m_Camera->GetPos();*/

        uint32_t uniformBuffersCount = 50;
        uint32_t uniformBufferChunkSize = 340;

        for (uint32_t i = 0; i < uniformBuffersCount; i++) {
            std::pair<VkBuffer, VkDeviceMemory> uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(uniformBufferChunkSize);
            mesh->m_UniformBuffers.emplace_back(uniformBuffer);
        }

        std::vector<VkDescriptorImageInfo> imageInfos;

        uint32_t index = 0;
        for (Data& data : *mesh->GetData()) {

            data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(*m_Adapter->GetEntitiesCommandPool(), data.m_Vertices);
            data.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(*m_Adapter->GetEntitiesCommandPool(), data.m_Indices);
            data.m_TextureIndex = index;

            Texture tex = m_Adapter->GetTextureManager()->GetTextures()[data.m_Texture];

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = tex.imageView;
            imageInfo.sampler = tex.sampler;

            imageInfos.emplace_back(imageInfo);
            index++;
        }

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = index;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayout desriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(
            bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        m_Adapter->GetDescriptorSetLayouts()->emplace_back(desriptorSetLayout);

        for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
            VkDescriptorSet meshDescriptorSets = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets(mesh->m_UniformBuffers, meshDescriptorSets, imageInfos);

            mesh->m_DescriptorSets.emplace_back(meshDescriptorSets);
        }

        std::vector<VkPushConstantRange> pushConstants = {};
        VkPushConstantRange vkPushconstants;
        vkPushconstants.offset = 0;
        vkPushconstants.size = sizeof(constants);
        vkPushconstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pushConstants.emplace_back(vkPushconstants);

        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, { desriptorSetLayout }, pushConstants);

        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;
        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();

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

        auto desc = Vertex::GetAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = desc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->m_PipelineCache,
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            false, true, true, true, false
        );
    }

    void VisitorVulkan::VisitDraw(Mesh* entity)
    {

    }
}