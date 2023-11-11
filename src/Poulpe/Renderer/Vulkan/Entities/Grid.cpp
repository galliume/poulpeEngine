#include "Grid.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct cPC;

    Grid::Grid(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager,
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_ShaderManager(shaderManager),
        m_TextureManager(textureManager),
        m_DescriptorPool(descriptorPool)
    {

    }

    void Grid::visit(Entity* entity)
    {
        Mesh* mesh = dynamic_cast<Mesh*>(entity);

        if (!mesh && !mesh->isDirty()) return;

        std::vector<Vertex> const vertices = {
            {{-1.f, -1.f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.f, -1.f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        std::vector<uint32_t> const indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        //ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        Data gridData;
        gridData.m_Texture = "grid";
        gridData.m_TextureIndex = 0;
        gridData.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        gridData.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        gridData.m_Ubos.emplace_back(ubo);
        gridData.m_Indices = indices;
        gridData.m_Vertices = vertices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("grid");
        mesh->setShaderName("grid");

        Buffer gridUniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);
        mesh->getUniformBuffers().emplace_back(gridUniformBuffer);

        Texture ctex = m_TextureManager->getTextures()["minecraft_grass"];

        std::vector<VkDescriptorImageInfo>cimageInfos{};
        VkDescriptorImageInfo cimageInfo{};
        cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo.imageView = ctex.getImageView();
        cimageInfo.sampler = ctex.getSampler();

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

        VkDescriptorSetLayout cdesriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(cbindings);

        m_Adapter->getDescriptorSetLayouts()->emplace_back(cdesriptorSetLayout);

        VkDescriptorSet cdescriptorSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { cdesriptorSetLayout }, 1);
        m_Adapter->rdr()->pdateDescriptorSets(mesh->getUniformBuffers(), cdescriptorSet, cimageInfos);
        mesh->getDescriptorSets().emplace_back(cdescriptorSet);

        Grid::pc pc;
        pc.point = glm::vec4(0.1f, 50.f, 0.f, 0.f);
        pc.view = m_Adapter->getCamera()->lookAt();

        mesh->applyPushConstants = [&pc](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* adapter, [[maybe_unused]] Data * data) {
            pc.point = glm::vec4(0.1f, 50.f, 0.f, 0.f);
            pc.view = adapter->getCamera()->lookAt();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Grid::pc), &pc);
        };
        mesh->setHasPushConstants();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(Grid::pc);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPcs.emplace_back(vkPc);

        std::vector<VkDescriptorSetLayout>dSetLayout = { cdesriptorSetLayout };

        mesh->setPipelineLayout(m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs));

        std::vector<VkPipelineShaderStageCreateInfo>cshadersStageInfos;

        VkPipelineShaderStageCreateInfo cvertShaderStageInfo{};
        cvertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cvertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        cvertShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[mesh->getShaderName()][0];
        cvertShaderStageInfo.pName = "main";
        cshadersStageInfos.emplace_back(cvertShaderStageInfo);

        VkPipelineShaderStageCreateInfo cfragShaderStageInfo{};
        cfragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cfragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        cfragShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[mesh->getShaderName()][1];
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

        mesh->setGraphicsPipeline(m_Adapter->rdr()->createGraphicsPipeline(m_Adapter->rdrPass(), mesh->getPipelineLayout(),
            mesh->getShaderName(), cshadersStageInfos, vertexInputInfo, VK_CULL_MODE_NONE, true, true, true, true,
            VulkanAdapter::s_PolygoneMode));


        for (uint32_t i = 0; i < mesh->getUniformBuffers().size(); i++) {
            //gridData.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            gridData.m_Ubos[i].proj = m_Adapter->getPerspective();

            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()[i], gridData.m_Ubos);
        }
        mesh->setData(gridData);
    }
}
