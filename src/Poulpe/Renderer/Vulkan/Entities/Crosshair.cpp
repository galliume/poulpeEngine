#include "Crosshair.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
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

    void Crosshair::visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->isDirty()) return;

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
        //ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        Data crossHairData;
        crossHairData.m_Texture = "crosshair";
        crossHairData.m_TextureIndex = 0;
        crossHairData.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        crossHairData.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        crossHairData.m_Ubos.emplace_back(ubo);
        crossHairData.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("crosshair");
        mesh->setShaderName("2d");

        Buffer crossHairuniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(crossHairuniformBuffer);

        Texture ctex = m_TextureManager->getTextures()["crosshair_1"];
        Texture ctex2 = m_TextureManager->getTextures()["crosshair_2"];

        std::vector<VkDescriptorImageInfo>cimageInfos;
        VkDescriptorImageInfo cimageInfo{};
        cimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo.imageView = ctex.getImageView();
        cimageInfo.sampler = ctex.getSampler();

        VkDescriptorImageInfo cimageInfo2{};
        cimageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cimageInfo2.imageView = ctex2.getImageView();
        cimageInfo2.sampler = ctex2.getSampler();

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

        VkDescriptorSetLayout cdesriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(cbindings);

        m_Adapter->getDescriptorSetLayouts()->emplace_back(cdesriptorSetLayout);

        VkDescriptorSet cdescriptorSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { cdesriptorSetLayout }, 1);
        m_Adapter->rdr()->pdateDescriptorSets(mesh->m_UniformBuffers, cdescriptorSet, cimageInfos);
        mesh->m_DescriptorSets.emplace_back(cdescriptorSet);

        Crosshair::pc pc;
        pc.textureID = 0;

        mesh->applyPushConstants = [&pc](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout,  [[maybe_unused]] std::shared_ptr<VulkanAdapter> adapter,  [[maybe_unused]] Data& data) {
            pc.textureID = VulkanAdapter::s_Crosshair;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Crosshair::pc), &pc);
        };
        mesh->setHasPushConstants();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(Crosshair::pc);
        vkPc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);

        std::vector<VkDescriptorSetLayout>dSetLayout = { cdesriptorSetLayout };

        mesh->m_PipelineLayout = m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs);

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
        auto crossDesc = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = crossDesc.data();

        mesh->m_GraphicsPipeline = m_Adapter->rdr()->createGraphicsPipeline(
            m_Adapter->rdrPass(),
            mesh->m_PipelineLayout,
            mesh->getShaderName(),
            cshadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_FRONT_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
            //crossHairData.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            crossHairData.m_Ubos[i].proj = m_Adapter->getPerspective();

            m_Adapter->rdr()->updateUniformBuffer(
                mesh->m_UniformBuffers[i],
                crossHairData.m_Ubos
            );
        }

        mesh->setData(crossHairData);
    }
}
