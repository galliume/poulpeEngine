#include "rebulkpch.h"
#include "Skybox.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    struct constants;

    Skybox::Skybox(
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

    void Skybox::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->IsDirty()) return;

        const std::vector<Vertex> skyVertices = {
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
        ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.proj = m_Adapter->GetPerspective();

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

        Data data;
        data.m_Texture = "skybox";
        data.m_Vertices = skyVertices;
        data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, skyVertices);
        data.m_Ubos.emplace_back(ubo);
        data.m_TextureIndex = 0;

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);

        Buffer uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(1);
        mesh->m_UniformBuffers.emplace_back(uniformBuffer);

        Texture tex = m_TextureManager->GetSkyboxTexture();

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

        VkDescriptorSetLayout skyDesriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(
            skyBindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        );

        VkDescriptorImageInfo skyDescriptorImageInfo{};
        skyDescriptorImageInfo.sampler = tex.GetSampler();
        skyDescriptorImageInfo.imageView = tex.GetImageView();
        skyDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorSet skyDescriptorSet = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { skyDesriptorSetLayout }, 1);
        m_Adapter->Rdr()->UpdateDescriptorSets(mesh->m_UniformBuffers, skyDescriptorSet, { skyDescriptorImageInfo });
        mesh->m_DescriptorSets.emplace_back(skyDescriptorSet);

        constants pushConstants;
        pushConstants.data = glm::vec4(0.f, Rbk::VulkanAdapter::s_AmbiantLight.load(), Rbk::VulkanAdapter::s_FogDensity.load(), 0.f);
        pushConstants.cameraPos = m_Adapter->GetCamera()->GetPos();
        pushConstants.fogColor = glm::vec4({ Rbk::VulkanAdapter::s_FogColor[0].load(), Rbk::VulkanAdapter::s_FogColor[1].load(), Rbk::VulkanAdapter::s_FogColor[2].load(), 0.f });
        pushConstants.lightPos = glm::vec4(m_Adapter->GetLights().at(0), 0.f);
        pushConstants.view = m_Adapter->GetCamera()->LookAt();

        mesh->ApplyPushConstants = [=, &pushConstants, &mesh](VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, std::shared_ptr<VulkanAdapter> adapter, const Data& data) {
            pushConstants.data = glm::vec4(data.m_TextureIndex * 1.f, Rbk::VulkanAdapter::s_AmbiantLight.load(), Rbk::VulkanAdapter::s_FogDensity.load(), 0.f);
            pushConstants.cameraPos = adapter->GetCamera()->GetPos();
            pushConstants.fogColor = glm::vec4({ Rbk::VulkanAdapter::s_FogColor[0].load(), Rbk::VulkanAdapter::s_FogColor[1].load(), Rbk::VulkanAdapter::s_FogColor[2].load(), 0.f });
            pushConstants.lightPos = glm::vec4(adapter->GetLights().at(0), 0.f);
            pushConstants.view = adapter->GetCamera()->LookAt();
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstants), &pushConstants);
        };

        mesh->SetHasPushConstants();

        std::vector<VkDescriptorSetLayout>dSetLayout = { skyDesriptorSetLayout };

        std::vector<VkPushConstantRange> vkPcs = {};
        VkPushConstantRange vkPc;
        vkPc.offset = 0;
        vkPc.size = sizeof(constants);
        vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        vkPcs.emplace_back(vkPc);
        mesh->m_PipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(mesh->m_DescriptorSets, dSetLayout, vkPcs);
        std::string shaderName = "skybox";

        VkVertexInputBindingDescription bDesc = Vertex::GetBindingDescription();
        std::vector<VkPipelineShaderStageCreateInfo>shadersStageInfos;

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[shaderName][0];
        vertShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(vertShaderStageInfo);

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_ShaderManager->GetShaders()->shaders[shaderName][1];
        fragShaderStageInfo.pName = "main";
        shadersStageInfos.emplace_back(fragShaderStageInfo);

        auto skyDesc = Vertex::GetAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
        vertexInputInfo.pVertexBindingDescriptions = &bDesc;
        vertexInputInfo.pVertexAttributeDescriptions = skyDesc.data();

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
            shadersStageInfos,
            vertexInputInfo,
            VK_CULL_MODE_NONE,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        glm::mat4 skybowView = m_Adapter->GetCamera()->LookAt();

        for (uint32_t i = 0; i < mesh->m_UniformBuffers.size(); i++) {
            data.m_Ubos[i].view = skybowView;
            m_Adapter->Rdr()->UpdateUniformBuffer(
                mesh->m_UniformBuffers[i],
                data.m_Ubos,
                1
            );
        }

        mesh->GetData()->emplace_back(data);
        mesh->SetIsDirty(false);
    }
}
