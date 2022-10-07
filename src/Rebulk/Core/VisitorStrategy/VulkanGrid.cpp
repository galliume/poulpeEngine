#include "rebulkpch.h"
#include "VulkanGrid.h"

namespace Rbk
{
    struct cPC;

    VulkanGrid::VulkanGrid(
        std::shared_ptr<VulkanAdapter> adapter,
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_DescriptorPool(descriptorPool)
    {

    }

    void VulkanGrid::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);

        if (!mesh) return;

        const std::vector<Vertex2D> vertices = {
            {{-0.025f, -0.025f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.025f, -0.025f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.025f, 0.025f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.025f, 0.025f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        Data crossHairData;
        crossHairData.m_Texture = "grid";
        crossHairData.m_TextureIndex = 0;
        crossHairData.m_VertexBuffer = m_Adapter->Rdr()->CreateVertex2DBuffer(commandPool, vertices);
        crossHairData.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        crossHairData.m_Ubos.emplace_back(ubo);
        crossHairData.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        mesh->SetName("grid");
        mesh->SetShaderName("grid");

        std::pair<VkBuffer, VkDeviceMemory> gridUniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(gridUniformBuffer);

        Texture ctex = m_Adapter->GetTextureManager()->GetTextures()["minecraft_grass"];

        std::vector<VkDescriptorImageInfo>cimageInfos{};
        VkDescriptorImageInfo cimageInfo{};
        cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo.imageView = ctex.GetImageView();
        cimageInfo.sampler = ctex.GetSampler();

        cimageInfos.emplace_back(cimageInfo);

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

        std::vector<VkDescriptorSetLayout>dSetLayout = { cdesriptorSetLayout };

        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, dSetLayout, {});

        std::vector<VkPipelineShaderStageCreateInfo>cshadersStageInfos;

        VkPipelineShaderStageCreateInfo cvertShaderStageInfo{};
        cvertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cvertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        cvertShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders[mesh->GetShaderName()][0];
        cvertShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cvertShaderStageInfo);

        VkPipelineShaderStageCreateInfo cfragShaderStageInfo{};
        cfragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cfragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        cfragShaderStageInfo.module = m_Adapter->GetShaderManager()->GetShaders()->shaders[mesh->GetShaderName()][1];
        cfragShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cfragShaderStageInfo);

        VkVertexInputBindingDescription bDesc = Vertex2D::GetBindingDescription();
        auto gridDesc = Vertex2D::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo2D{};
        vertexInputInfo2D.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo2D.vertexBindingDescriptionCount = 1;
        vertexInputInfo2D.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2D::GetAttributeDescriptions().size());
        vertexInputInfo2D.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo2D.pVertexAttributeDescriptions = gridDesc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
            cshadersStageInfos,
            vertexInputInfo2D,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        mesh->GetData()->emplace_back(crossHairData);
    }
}
