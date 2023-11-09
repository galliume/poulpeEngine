#include "Splash.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    Splash::Splash(
         std::shared_ptr<VulkanAdapter> adapter,
         std::shared_ptr<EntityManager> entityManager,
         std::shared_ptr<ShaderManager> shaderManager,
         std::shared_ptr<TextureManager> textureManager,
         std::shared_ptr<SpriteAnimationManager> spriteAnimationManager,
         VkDescriptorPool descriptorPool) :
         m_Adapter(adapter),
         m_EntityManager(entityManager),
         m_ShaderManager(shaderManager),
         m_TextureManager(textureManager),
         m_SpriteAnimationManager(spriteAnimationManager),
         m_DescriptorPool(descriptorPool)
    {

    }

    void Splash::visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
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

        Data data;
        data.m_Texture = "splashscreen";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertex2DBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        data.m_Ubos.emplace_back(ubo);
        data.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("splashscreen");
        mesh->setShaderName("splashscreen");

        Buffer uniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(uniformBuffer);

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
        m_Adapter->rdr()->pdateDescriptorSets(mesh->m_UniformBuffers, descriptorSet, imageInfos);
        mesh->m_DescriptorSets.emplace_back(descriptorSet);

        Splash::pc pc;
        pc.textureID = 0;

        mesh->applyPushConstants = [&pc, mesh](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, [[maybe_unused]] std::shared_ptr<VulkanAdapter> adapter, [[maybe_unused]] Data& data) {
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

        mesh->m_PipelineLayout = m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs);

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

        mesh->m_GraphicsPipeline = m_Adapter->rdr()->createGraphicsPipeline(
            m_Adapter->rdrPass(),
            mesh->m_PipelineLayout,
            mesh->getShaderName(),
            shadersStageInfos,
            vertexInputInfo2D,
            VK_CULL_MODE_NONE,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
            //data.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            data.m_Ubos[i].proj = m_Adapter->getPerspective();
        }

        for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
            m_Adapter->rdr()->updateUniformBuffer(
                mesh->m_UniformBuffers[i],
                data.m_Ubos
            );
        }
        
        mesh->setData(data);
    }
}
