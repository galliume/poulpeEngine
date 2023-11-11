#include "Skybox.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct constants;

    Skybox::Skybox(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager) :
        m_Adapter(adapter),
        m_ShaderManager(shaderManager),
        m_TextureManager(textureManager)
    {

    }

    void Skybox::visit(Entity* entity)
    {
        Mesh* mesh = dynamic_cast<Mesh*>(entity);

        if (!mesh && !mesh->isDirty()) return;

        std::vector<Vertex> const skyVertices = {
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        };

        UniformBufferObject ubo;
        ubo.model = glm::mat4(0.0f);
        //ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.proj = m_Adapter->getPerspective();

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        Data data;
        data.m_Texture = "skybox";
        data.m_Vertices = skyVertices;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, skyVertices);
        data.m_Ubos.emplace_back(ubo);
        data.m_TextureIndex = 0;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        Buffer uniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);

        setPushConstants(mesh);

        mesh->setDescriptorSetLayout(createDescriptorSetLayout());
        mesh->setPipelineLayout(createPipelineLayout(mesh->getDescriptorSetLayout()));
        mesh->setDescriptorSets(createDescriptorSet(mesh));

        auto shaders = getShaders("skybox");

        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto  vertexInputInfo = getVertexBindingDesc(bDesc, attDesc);

        mesh->setGraphicsPipeline(m_Adapter->rdr()->createGraphicsPipeline(m_Adapter->rdrPass(), mesh->getPipelineLayout(),
            mesh->getShaderName(), shaders, vertexInputInfo, VK_CULL_MODE_NONE, true, true, true, true,
            VulkanAdapter::s_PolygoneMode));

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), & data.m_Ubos);
        }

        mesh->setData(data);
        mesh->setIsDirty(false);
    }

    VkDescriptorSetLayout Skybox::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding skySamplerLayoutBinding{};
        skySamplerLayoutBinding.binding = 1;
        skySamplerLayoutBinding.descriptorCount = 1;
        skySamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skySamplerLayoutBinding.pImmutableSamplers = nullptr;
        skySamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayoutBinding> skyBindings = { uboLayoutBinding, skySamplerLayoutBinding };

        VkDescriptorSetLayout desriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(skyBindings);

        return desriptorSetLayout;
    }
    
    std::vector<VkDescriptorSet> Skybox::createDescriptorSet(Mesh* mesh)
    {
        if (!mesh->getDescriptorSets().empty()) {
            vkFreeDescriptorSets(m_Adapter->rdr()->getDevice(), mesh->getDescriptorPool(),
                mesh->getDescriptorSets().size(), mesh->getDescriptorSets().data());

            mesh->getDescriptorSets().clear();
        }

        Texture tex = m_TextureManager->getSkyboxTexture();

        VkDescriptorImageInfo descriptorImageInfo{};
        descriptorImageInfo.sampler = tex.getSampler();
        descriptorImageInfo.imageView = tex.getImageView();
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorSet descriptorSet = m_Adapter->rdr()->createDescriptorSets(mesh->getDescriptorPool(),
            {mesh->getDescriptorSetLayout()}, 1);

        std::vector<VkDescriptorImageInfo> descriptorImageInfos{};
        descriptorImageInfos.emplace_back(descriptorImageInfo);

        m_Adapter->rdr()->updateDescriptorSets(*mesh->getUniformBuffers(), descriptorSet, descriptorImageInfos);

        return { descriptorSet };
    }

    VkPipelineLayout Skybox::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
    {
        std::vector<VkDescriptorSetLayout> dSetLayout = { descriptorSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPcs.emplace_back(vkPc);

        VkPipelineLayout pipelineLayout = m_Adapter->rdr()->createPipelineLayout(dSetLayout, vkPcs);

        return pipelineLayout;
    }

    std::vector<VkPipelineShaderStageCreateInfo> Skybox::getShaders(std::string const & name)
    {
        std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos{};

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

    void Skybox::setPushConstants(Mesh* mesh)
    {
        constants pushConstants{};
        pushConstants.data = glm::vec4(0.f, Poulpe::VulkanAdapter::s_AmbiantLight.load(),
            Poulpe::VulkanAdapter::s_FogDensity.load(), 0.f);

        pushConstants.cameraPos = m_Adapter->getCamera()->getPos();

        pushConstants.fogColor = glm::vec4({ Poulpe::VulkanAdapter::s_FogColor[0].load(), Poulpe::VulkanAdapter::s_FogColor[1].load(),
            Poulpe::VulkanAdapter::s_FogColor[2].load(), 0.f });

        pushConstants.lightPos = glm::vec4(m_Adapter->getLights().at(0), 0.f);
        pushConstants.view = m_Adapter->getCamera()->lookAt();

        mesh->applyPushConstants = [=, &pushConstants](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* adapter,  [[maybe_unused]] Data * data) {

            pushConstants.data = glm::vec4(0.f, Poulpe::VulkanAdapter::s_AmbiantLight.load(),
                Poulpe::VulkanAdapter::s_FogDensity.load(), 0.f);

            pushConstants.cameraPos = adapter->getCamera()->getPos();
            pushConstants.fogColor = glm::vec4({ Poulpe::VulkanAdapter::s_FogColor[0].load(),
                Poulpe::VulkanAdapter::s_FogColor[1].load(), Poulpe::VulkanAdapter::s_FogColor[2].load(), 0.f });

            pushConstants.lightPos = glm::vec4(adapter->getLights().at(0), 0.f);
            pushConstants.view = glm::mat4(glm::mat3(adapter->getCamera()->lookAt()));

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants),
                & pushConstants);
        };

        mesh->setHasPushConstants();
    }

    VkPipelineVertexInputStateCreateInfo Skybox::getVertexBindingDesc(VkVertexInputBindingDescription bDesc, 
      std::array<VkVertexInputAttributeDescription, 3> attDesc)
    {
      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
      vertexInputInfo.pVertexBindingDescriptions = & bDesc;
      vertexInputInfo.pVertexAttributeDescriptions = attDesc.data();

      return vertexInputInfo;
    }
}
