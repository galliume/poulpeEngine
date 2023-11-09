#include "Basic.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct constants;

    Basic::Basic(
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

    void Basic::visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->isDirty()) return;

        uint32_t totalInstances = static_cast<uint32_t>(mesh->getData()->m_Ubos.size());
        uint32_t maxUniformBufferRange = m_Adapter->rdr()->getDeviceProperties().limits.maxUniformBufferRange;
        uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));

        //@todo fix memory management...
        uint32_t uboOffset = (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances;
        uint32_t uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        uint32_t nbUbo = uboOffset;

        for (size_t i = 0; i < uniformBuffersCount; ++i) {

          mesh->getData()->m_UbosOffset.emplace_back(uboOffset);
          Buffer uniformBuffer = m_Adapter->rdr()->createUniformBuffers(nbUbo);
          mesh->m_UniformBuffers.emplace_back(uniformBuffer);

          uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
          nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
          uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        }

        auto commandPool = m_Adapter->rdr()->createCommandPool();
        auto data = mesh->getData();

        data->m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, data->m_Vertices);
        data->m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, data->m_Indices);
        data->m_TextureIndex = 0;

        for (size_t i = 0; i < mesh->getData()->m_Ubos.size(); ++i) {
          mesh->getData()->m_Ubos[i].proj = m_Adapter->getPerspective();
        }

        VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout();
        std::vector<VkDescriptorSet> desriptorSets = createDescriptorSet(mesh, descriptorSetLayout);
        VkPipelineLayout pipelineLayout = createPipelineLayout(descriptorSetLayout);

        m_Adapter->getDescriptorSetLayouts()->emplace_back(descriptorSetLayout);

        mesh->m_DescriptorSetLayout = descriptorSetLayout;
        mesh->m_DescriptorSets = desriptorSets;
        mesh->m_PipelineLayout = pipelineLayout;

        int min{ 0 };
        int max{ 0 };

        for (size_t i = 0; i < mesh->m_UniformBuffers.size(); ++i) {
          max = mesh->getData()->m_UbosOffset.at(i);
          auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);

          m_Adapter->rdr()->updateUniformBuffer(
            mesh->m_UniformBuffers[i],
            ubos
          );

          min = max;
        }


        setPushConstants(mesh);

        auto shaders = getShaders(mesh->getShaderName());

        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attDesc);

        mesh->m_GraphicsPipeline = m_Adapter->rdr()->createGraphicsPipeline(
            m_Adapter->rdrPass(),
            mesh->m_PipelineLayout,
            mesh->getShaderName(),
          shaders,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        if (m_Adapter->getDrawBbox() && mesh->hasBbox()) {
            createBBoxEntity(mesh);
            mesh->setHasBbox(true);
        }

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);
    }

    void Basic::createBBoxEntity(std::shared_ptr<Mesh>& mesh)
    {
        Poulpe::Entity::BBox* box = mesh->getBBox().get();

        UniformBufferObject ubo;
        glm::mat4 transform = glm::translate(glm::mat4(1), box->center) * glm::scale(glm::mat4(1), box->size);
        ubo.model = box->position * transform;

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        const std::vector<Vertex> vertices = {
            {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0, //front
            1, 5, 6, 6, 2, 1, // right
            7, 6, 5, 5, 4, 7, // back
            4, 0, 3, 3, 7, 4, // left
            4, 5, 1, 1, 0, 4, // bottom
            3, 2, 6, 6, 7, 3 // top
        };

        Data data;
        data.m_Texture = "minecraft_grass";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        data.m_Indices = indices;
        data.m_Vertices = vertices;
        data.m_Ubos.emplace_back(ubo);

        if (box->mesh->getData()->m_Ubos.size() > 0)
            data.m_Ubos.insert(data.m_Ubos.end(), box->mesh->getData()->m_Ubos.begin(), box->mesh->getData()->m_Ubos.end());

        for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
            data.m_Ubos[i].proj = m_Adapter->getPerspective();
        }

        box->mesh->setName("bbox_" + mesh->getData()->m_Name);
        box->mesh->setShaderName("bbox");
        box->mesh->setData(data);

        VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout();
        std::vector<VkDescriptorSet> desriptorSets = createDescriptorSet(box->mesh, descriptorSetLayout);
        VkPipelineLayout pipelineLayout = createPipelineLayout(descriptorSetLayout);

        box->mesh->m_DescriptorSetLayout = descriptorSetLayout;
        box->mesh->m_DescriptorSets = desriptorSets;
        box->mesh->m_PipelineLayout = pipelineLayout;

        setPushConstants(box->mesh);

        auto shaders = Basic::getShaders("bbox");
        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attDesc);

        box->mesh->m_GraphicsPipeline = m_Adapter->rdr()->createGraphicsPipeline(
            m_Adapter->rdrPass(),
            box->mesh->m_PipelineLayout,
            box->mesh->getShaderName(),
            shaders,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VK_POLYGON_MODE_LINE
        );

        for (uint32_t i = 0; i < box->mesh->m_UniformBuffers.size(); i++) {
            m_Adapter->rdr()->updateUniformBuffer(
                box->mesh->m_UniformBuffers[i],
                box->mesh->getData()->m_Ubos
            );
        }
    }

    VkDescriptorSetLayout Basic::createDescriptorSetLayout()
    {
      VkDescriptorSetLayoutBinding uboLayoutBinding{};
      uboLayoutBinding.binding = 0;
      uboLayoutBinding.descriptorCount = 1;
      uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      uboLayoutBinding.pImmutableSamplers = nullptr;
      uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkDescriptorSetLayoutBinding samplerLayoutBinding{};
      samplerLayoutBinding.binding = 1;
      samplerLayoutBinding.descriptorCount = 1;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr;
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

      VkDescriptorSetLayout desriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(bindings);

      return desriptorSetLayout;
    }

    std::vector<VkDescriptorSet> Basic::createDescriptorSet(std::shared_ptr<Mesh> mesh, VkDescriptorSetLayout descriptorSetLayout)
    {
      if (!mesh->m_DescriptorSets.empty()) {
        vkFreeDescriptorSets(m_Adapter->rdr()->getDevice(), mesh->m_DescriptorPool, mesh->m_DescriptorSets.size(), mesh->m_DescriptorSets.data());
        mesh->m_DescriptorSets.clear();
      }

      std::vector<VkDescriptorImageInfo> imageInfos;

      Texture tex = m_TextureManager->getTextures()[mesh->getData()->m_Texture];

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = tex.getImageView();
      imageInfo.sampler = tex.getSampler();

      imageInfos.emplace_back(imageInfo);

      std::vector<VkDescriptorSet> descSets{};

      for (auto ubo : mesh->m_UniformBuffers) {
        VkDescriptorSet descSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { descriptorSetLayout }, 1);
        m_Adapter->rdr()->pdateDescriptorSets({ ubo }, descSet, imageInfos);
        for (uint32_t i = 0; i < m_Adapter->getSwapChainImages()->size(); i++) {
          descSets.emplace_back(descSet);
        }
      }

      return descSets;
    }

    VkPipelineLayout Basic::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
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

    std::vector<VkPipelineShaderStageCreateInfo> Basic::getShaders(std::string const & shaderName)
    {
      std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[shaderName][0];
      vertShaderStageInfo.pName = "main";
      shadersStageInfos.emplace_back(vertShaderStageInfo);

      VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
      fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = m_ShaderManager->getShaders()->shaders[shaderName][1];
      fragShaderStageInfo.pName = "main";
      shadersStageInfos.emplace_back(fragShaderStageInfo);

      return shadersStageInfos;
    }

    void Basic::setPushConstants(std::shared_ptr<Mesh> mesh)
    {
      constants pushConstants;
      pushConstants.data = glm::vec4(0.f, Poulpe::VulkanAdapter::s_AmbiantLight.load(), Poulpe::VulkanAdapter::s_FogDensity.load(), 0.f);
      pushConstants.cameraPos = m_Adapter->getCamera()->getPos();
      pushConstants.fogColor = glm::vec4({ Poulpe::VulkanAdapter::s_FogColor[0].load(), Poulpe::VulkanAdapter::s_FogColor[1].load(), Poulpe::VulkanAdapter::s_FogColor[2].load(), 0.f });
      pushConstants.lightPos = glm::vec4(m_Adapter->getLights().at(0), 0.f);
      pushConstants.view = m_Adapter->getCamera()->lookAt();
      pushConstants.ambiantLight = Poulpe::VulkanAdapter::s_AmbiantLight.load();
      pushConstants.fogDensity = Poulpe::VulkanAdapter::s_FogDensity.load();

      mesh->applyPushConstants = [=, &pushConstants](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter> adapter, const Data& data) {
        pushConstants.data = glm::vec4(static_cast<float>(data.m_TextureIndex), Poulpe::VulkanAdapter::s_AmbiantLight.load(), Poulpe::VulkanAdapter::s_FogDensity.load(), 0.f);
        pushConstants.cameraPos = adapter->getCamera()->getPos();
        pushConstants.fogColor = glm::vec4({ Poulpe::VulkanAdapter::s_FogColor[0].load(), Poulpe::VulkanAdapter::s_FogColor[1].load(), Poulpe::VulkanAdapter::s_FogColor[2].load(), 0.f });
        pushConstants.lightPos = glm::vec4(adapter->getLights().at(0), 0.f);
        pushConstants.view = adapter->getCamera()->lookAt();
        pushConstants.ambiantLight = Poulpe::VulkanAdapter::s_AmbiantLight.load();
        pushConstants.fogDensity = Poulpe::VulkanAdapter::s_FogDensity.load();
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
      };

      mesh->setHasPushConstants();
    }

    VkPipelineVertexInputStateCreateInfo Basic::getVertexBindingDesc(VkVertexInputBindingDescription bDesc,
      std::array<VkVertexInputAttributeDescription, 3> attDesc)
    {
      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size());
      vertexInputInfo.pVertexBindingDescriptions = &bDesc;
      vertexInputInfo.pVertexAttributeDescriptions = attDesc.data();

      return vertexInputInfo;
    }
}
