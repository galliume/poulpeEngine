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

        Buffer crossHairuniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);

        mesh->setName("crosshair");
        mesh->setShaderName("2d");
        mesh->m_UniformBuffers.emplace_back(crossHairuniformBuffer);
        mesh->m_DescriptorSetLayout = createDescriptorSetLayout();
        mesh->m_DescriptorSets = createDescriptorSet(mesh);
        mesh->m_PipelineLayout = createPipelineLayout(mesh->m_DescriptorSetLayout);

        setPushConstants(mesh);

        m_Adapter->getDescriptorSetLayouts()->emplace_back(mesh->m_DescriptorSetLayout);

        auto shaders = getShaders(mesh->getShaderName());
        auto bDesc = Vertex::GetBindingDescription();
        auto attrDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attrDesc);

        mesh->m_GraphicsPipeline = m_Adapter->rdr()->createGraphicsPipeline(
            m_Adapter->rdrPass(),
            mesh->m_PipelineLayout,
            mesh->getShaderName(),
            shaders,
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
    VkDescriptorSetLayout Crosshair::createDescriptorSetLayout()
    {
         VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 2;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

         VkDescriptorSetLayout desriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(bindings);

         return desriptorSetLayout;
    }

    std::vector<VkDescriptorSet> Crosshair::createDescriptorSet(std::shared_ptr<Mesh> mesh)
    {
        Texture tex = m_TextureManager->getTextures()["crosshair_1"];
        Texture tex2 = m_TextureManager->getTextures()["crosshair_2"];

        std::vector<VkDescriptorImageInfo> imageInfos;
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex.getImageView();
        imageInfo.sampler = tex.getSampler();

        VkDescriptorImageInfo imageInfo2{};
        imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo2.imageView = tex2.getImageView();
        imageInfo2.sampler = tex2.getSampler();

        imageInfos.emplace_back(imageInfo);
        imageInfos.emplace_back(imageInfo2);

        VkDescriptorSet descriptorSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { mesh->m_DescriptorSetLayout }, 1);
        m_Adapter->rdr()->pdateDescriptorSets(mesh->m_UniformBuffers, descriptorSet, imageInfos);

        return { descriptorSet };
    }

    VkPipelineLayout Crosshair::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
    {
        std::vector<VkDescriptorSetLayout>dSetLayout = { descriptorSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(Crosshair::pc);
        vkPc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);


        VkPipelineLayout pipelineLayout = m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs);

        return pipelineLayout;
    }

    std::vector<VkPipelineShaderStageCreateInfo> Crosshair::getShaders(std::string const & name)
    {
        std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[name][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[name][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        return shadersStageInfos;
    }

    VkPipelineVertexInputStateCreateInfo Crosshair::getVertexBindingDesc(VkVertexInputBindingDescription bDesc, std::array<VkVertexInputAttributeDescription, 3> attDesc)
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = attDesc.data();

        return vertexInputInfo;
    }

    void Crosshair::setPushConstants(std::shared_ptr<Mesh> mesh)
    {
        Crosshair::pc pc;
        pc.textureID = 0;

        mesh->applyPushConstants = [&pc](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout,  [[maybe_unused]] std::shared_ptr<VulkanAdapter> adapter,  [[maybe_unused]] Data& data) {
            pc.textureID = VulkanAdapter::s_Crosshair;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Crosshair::pc), &pc);
        };
        mesh->setHasPushConstants();
    }
}
