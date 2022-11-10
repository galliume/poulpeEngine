#include "rebulkpch.h"
#include "Grid.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    struct cPC;

    Grid::Grid(
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

    void Grid::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->IsDirty()) return;

        const std::vector<Vertex> vertices = {
            {{-1.f, -1.f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.f, -1.f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        Data gridData;
        gridData.m_Texture = "grid";
        gridData.m_TextureIndex = 0;
        gridData.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, vertices);
        gridData.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        gridData.m_Ubos.emplace_back(ubo);
        gridData.m_Indices = indices;
        gridData.m_Vertices = vertices;

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        mesh->SetName("grid");
        mesh->SetShaderName("grid");

        Buffer gridUniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(gridUniformBuffer);

        Texture ctex = m_TextureManager->GetTextures()["minecraft_grass"];

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

        VkDescriptorSet cdescriptorSet = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { cdesriptorSetLayout }, 1);
        m_Adapter->Rdr()->UpdateDescriptorSets(mesh->m_UniformBuffers, cdescriptorSet, cimageInfos);
        mesh->m_DescriptorSets.emplace_back(cdescriptorSet);

        Grid::pc pc;
        pc.point = glm::vec4(0.1f, 50.f, 0.f, 0.f);
        pc.view = m_Adapter->GetCamera()->LookAt();

        mesh->ApplyPushConstants = [&pc](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter> adapter, Data& data) {
            pc.point = glm::vec4(0.1f, 50.f, 0.f, 0.f);
            pc.view = adapter->GetCamera()->LookAt();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Grid::pc), &pc);
        };
        mesh->SetHasPushConstants();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(Grid::pc);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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
        auto gridDesc = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = gridDesc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
            cshadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_NONE,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );


        for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
            gridData.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            gridData.m_Ubos[i].proj = m_Adapter->GetPerspective();

            m_Adapter->Rdr()->UpdateUniformBuffer(
                mesh->m_UniformBuffers[i],
                gridData.m_Ubos,
                gridData.m_Ubos.size()
            );
        }

        mesh->GetData()->emplace_back(gridData);
    }
}
