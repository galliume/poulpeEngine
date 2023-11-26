#include "Splash.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    Splash::Splash(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager,
         SpriteAnimationManager* spriteAnimationManager, VkDescriptorPool descriptorPool) :
         m_Adapter(adapter),
         m_ShaderManager(shaderManager),
         m_TextureManager(textureManager),
         m_SpriteAnimationManager(spriteAnimationManager),
         m_DescriptorPool(descriptorPool)
    {

    }

    void Splash::visit(Mesh* mesh)
    {
        if (!mesh && !mesh->isDirty()) return;

        const std::vector<Vertex2D> vertices = {
            {{0.5f, -0.6f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.6f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, 0.6f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, 0.6f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        UniformBufferObject ubo;
        //ubo.view = glm::mat4(0.0f);

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        Mesh::Data data;
        data.m_Texture = "splashscreen";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertex2DBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        data.m_Ubos.emplace_back(ubo);
        data.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("splashscreen");
        mesh->setShaderName("splashscreen");

        Mesh::Buffer uniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);

        std::vector<VkDescriptorImageInfo> imageInfos{};
        auto sprites = m_SpriteAnimationManager->getSpritesByName("splashAnim");
        int index = 0;

        for (auto sprite : sprites) {

            std::string name = "splashAnim_" + std::to_string(index);
            m_TextureManager->addTexture(name, sprite);

            Texture texture = m_TextureManager->getTextures()[name];

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.getImageView();
            imageInfo.sampler = texture.getSampler();

            imageInfos.emplace_back(imageInfo);
            index++;
        }

        mesh->setSpritesCount(imageInfos.size());

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

        VkDescriptorSetLayout desriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(bindings);

        m_Adapter->getDescriptorSetLayouts()->emplace_back(desriptorSetLayout);

        VkDescriptorSet descriptorSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { desriptorSetLayout }, 1);
        m_Adapter->rdr()->updateDescriptorSets(*mesh->getUniformBuffers(), descriptorSet, imageInfos);
        mesh->getDescriptorSets().emplace_back(descriptorSet);

        Splash::pc pc;
        pc.textureID = 0;

        mesh->applyPushConstants = [&pc, mesh](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout, 
            [[maybe_unused]] VulkanAdapter* adapter, [[maybe_unused]] Mesh::Data * data) {
            pc.textureID = mesh->getNextSpriteIndex();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Splash::pc), &pc);
        };
        mesh->setHasPushConstants();

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(pc);
        vkPc.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        vkPcs.emplace_back(vkPc);

        std::vector<VkDescriptorSetLayout>dSetLayout = { desriptorSetLayout };

        mesh->setPipelineLayout(m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs));

        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[mesh->getShaderName()][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[mesh->getShaderName()][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        VkVertexInputBindingDescription bDesc2D = Vertex::GetBindingDescription();
        auto desc = Vertex2D::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo2D{};
        vertexInputInfo2D.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo2D.vertexBindingDescriptionCount = 1;
        vertexInputInfo2D.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex2D::getAttributeDescriptions().size());
        vertexInputInfo2D.pVertexBindingDescriptions = &bDesc2D;
        vertexInputInfo2D.pVertexAttributeDescriptions = desc.data();

        mesh->setGraphicsPipeline(m_Adapter->rdr()->createGraphicsPipeline(m_Adapter->rdrPass(), mesh->getPipelineLayout(),
            mesh->getShaderName(), shadersStageInfos, vertexInputInfo2D, VK_CULL_MODE_NONE, true, true, true, true,
            VulkanAdapter::s_PolygoneMode));

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            //data.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            data.m_Ubos[i].proj = m_Adapter->getPerspective();
        }

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), & data.m_Ubos);
        }
        
        mesh->setData(data);
    }
}
