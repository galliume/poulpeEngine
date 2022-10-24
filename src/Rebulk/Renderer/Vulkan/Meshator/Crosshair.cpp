#include "rebulkpch.h"
#include "Crosshair.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    Crosshair::Crosshair(
         std::shared_ptr<VulkanAdapter> adapter,
         std::shared_ptr<EntityManager> entityManager,
         std::shared_ptr<ShaderManager> shaderManager,
         std::shared_ptr<TextureManager> textureManager,
         VkDescriptorPool descriptorPool) :
         m_Adapter(adapter),
         m_EntityManager(entityManager),
         m_ShaderManager(shaderManager),
         m_TextureManager(textureManager),
         m_DescriptorPool(descriptorPool)
    {

    }

    void Crosshair::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->IsDirty()) return;

        const std::vector<Vertex> vertices = {
            {{-0.025f, -0.025f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.025f, -0.025f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.025f, 0.025f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.025f, 0.025f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        Data crossHairData;
        crossHairData.m_Texture = "crosshair";
        crossHairData.m_TextureIndex = 0;
        crossHairData.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, vertices);
        crossHairData.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        crossHairData.m_Ubos.emplace_back(ubo);
        crossHairData.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        mesh->SetName("crosshair");
        mesh->SetShaderName("2d");

        std::pair<VkBuffer, VkDeviceMemory> crossHairuniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(crossHairuniformBuffer);

        Texture ctex = m_TextureManager->GetTextures()["crosshair_1"];
        Texture ctex2 = m_TextureManager->GetTextures()["crosshair_2"];

        std::vector<VkDescriptorImageInfo>cimageInfos;
        VkDescriptorImageInfo cimageInfo{};
        cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo.imageView = ctex.GetImageView();
        cimageInfo.sampler = ctex.GetSampler();

        VkDescriptorImageInfo cimageInfo2{};
        cimageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo2.imageView = ctex2.GetImageView();
        cimageInfo2.sampler = ctex2.GetSampler();

        cimageInfos.emplace_back(cimageInfo);
        cimageInfos.emplace_back(cimageInfo2);

        VkDescriptorSetLayoutBinding cuboLayoutBinding{};
        cuboLayoutBinding.binding = 0;
        cuboLayoutBinding.descriptorCount = 1;
        cuboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        cuboLayoutBinding.pImmutableSamplers = nullptr;
        cuboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding csamplerLayoutBinding{};
        csamplerLayoutBinding.binding = 1;
        csamplerLayoutBinding.descriptorCount = cimageInfos.size();
        csamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        csamplerLayoutBinding.pImmutableSamplers = nullptr;
        csamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> cbindings = { cuboLayoutBinding, csamplerLayoutBinding };

        VkDescriptorSetLayout cdesriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(
            cbindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        m_Adapter->GetDescriptorSetLayouts()->emplace_back(cdesriptorSetLayout);

        for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
            VkDescriptorSet cdescriptorSet = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { cdesriptorSetLayout }, 1);
            m_Adapter->Rdr()->UpdateDescriptorSets(mesh->m_UniformBuffers, cdescriptorSet, cimageInfos);
            mesh->m_DescriptorSets.emplace_back(cdescriptorSet);
        }

        Crosshair::pc pc;
        pc.textureID = 0;

        mesh->ApplyPushConstants = [&pc](VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
            pc.textureID = VulkanAdapter::s_Crosshair;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        };
        mesh->SetHasPushConstants();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(pc);
        vkPc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);

        std::vector<VkDescriptorSetLayout>dSetLayout = { cdesriptorSetLayout };

        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, dSetLayout, vkPcs);

        std::vector<VkPipelineShaderStageCreateInfo>cshadersStageInfos;

        VkPipelineShaderStageCreateInfo cvertShaderStageInfo{};
        cvertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cvertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        cvertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[mesh->GetShaderName()][0];
        cvertShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cvertShaderStageInfo);

        VkPipelineShaderStageCreateInfo cfragShaderStageInfo{};
        cfragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cfragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        cfragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[mesh->GetShaderName()][1];
        cfragShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cfragShaderStageInfo);

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        auto crossDesc = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = crossDesc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
            cshadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_FRONT_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        mesh->GetData()->emplace_back(crossHairData);
    }
}
