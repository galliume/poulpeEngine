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

    void Basic::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh && !mesh->IsDirty()) return;

        uint32_t totalInstances = static_cast<uint32_t>(mesh->GetData()->m_Ubos.size());
        uint32_t maxUniformBufferRange = m_Adapter->Rdr()->GetDeviceProperties().limits.maxUniformBufferRange;
        uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
        uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));

        //@todo fix memory management...
        uint32_t uboOffset = (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances;
        uint32_t uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        uint32_t nbUbo = uboOffset;

        for (size_t i = 0; i < uniformBuffersCount; ++i) {

          mesh->GetData()->m_UbosOffset.emplace_back(uboOffset);
          Buffer uniformBuffer = m_Adapter->Rdr()->CreateUniformBuffers(nbUbo);
          mesh->m_UniformBuffers.emplace_back(uniformBuffer);

          uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
          nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
          uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        }

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();
        auto data = mesh->GetData();

        data->m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, data->m_Vertices);
        data->m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, data->m_Indices);
        data->m_TextureIndex = 0;

        for (size_t i = 0; i < mesh->GetData()->m_Ubos.size(); ++i) {
          mesh->GetData()->m_Ubos[i].proj = m_Adapter->GetPerspective();
        }

        VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout();
        std::vector<VkDescriptorSet> desriptorSets = CreateDescriptorSet(mesh, descriptorSetLayout);
        VkPipelineLayout pipelineLayout = CreatePipelineLayout(descriptorSetLayout);

        m_Adapter->GetDescriptorSetLayouts()->emplace_back(descriptorSetLayout);

        mesh->m_DescriptorSetLayout = descriptorSetLayout;
        mesh->m_DescriptorSets = desriptorSets;
        mesh->m_PipelineLayout = pipelineLayout;

        int min{ 0 };
        int max{ 0 };

        for (size_t i = 0; i < mesh->m_UniformBuffers.size(); ++i) {
          max = mesh->GetData()->m_UbosOffset.at(i);
          auto ubos = std::vector<UniformBufferObject>(mesh->GetData()->m_Ubos.begin() + min, mesh->GetData()->m_Ubos.begin() + max);

          m_Adapter->Rdr()->UpdateUniformBuffer(
            mesh->m_UniformBuffers[i],
            ubos
          );

          min = max;
        }


        SetPushConstants(mesh);

        auto shaders = GetShaders(mesh->GetShaderName());

        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = GetVertexBindingDesc(bDesc, attDesc);

        mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            mesh->m_PipelineLayout,
            mesh->GetShaderName(),
          shaders,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VulkanAdapter::s_PolygoneMode
        );

        if (m_Adapter->GetDrawBbox() && mesh->HasBbox()) {
            CreateBBoxEntity(mesh);
            mesh->SetHasBbox(true);
        }

        vkDestroyCommandPool(m_Adapter->Rdr()->GetDevice(), commandPool, nullptr);
    }

    void Basic::CreateBBoxEntity(std::shared_ptr<Mesh>& mesh)
    {
        Poulpe::Entity::BBox* box = mesh->GetBBox().get();

        UniformBufferObject ubo;
        glm::mat4 transform = glm::translate(glm::mat4(1), box->center) * glm::scale(glm::mat4(1), box->size);
        ubo.model = box->position * transform;

        auto commandPool = m_Adapter->Rdr()->CreateCommandPool();

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
        data.m_VertexBuffer = m_Adapter->Rdr()->CreateVertexBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->Rdr()->CreateIndexBuffer(commandPool, indices);
        data.m_Indices = indices;
        data.m_Vertices = vertices;
        data.m_Ubos.emplace_back(ubo);

        if (box->mesh->GetData()->m_Ubos.size() > 0)
            data.m_Ubos.insert(data.m_Ubos.end(), box->mesh->GetData()->m_Ubos.begin(), box->mesh->GetData()->m_Ubos.end());

        for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
            data.m_Ubos[i].proj = m_Adapter->GetPerspective();
        }

        box->mesh->SetName("bbox_" + mesh->GetData()->m_Name);
        box->mesh->SetShaderName("bbox");
        box->mesh->SetData(data);

        VkDescriptorSetLayout descriptorSetLayout = CreateDescriptorSetLayout();
        std::vector<VkDescriptorSet> desriptorSets = CreateDescriptorSet(box->mesh, descriptorSetLayout);
        VkPipelineLayout pipelineLayout = CreatePipelineLayout(descriptorSetLayout);

        box->mesh->m_DescriptorSetLayout = descriptorSetLayout;
        box->mesh->m_DescriptorSets = desriptorSets;
        box->mesh->m_PipelineLayout = pipelineLayout;

        SetPushConstants(box->mesh);

        auto shaders = Basic::GetShaders("bbox");
        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = GetVertexBindingDesc(bDesc, attDesc);

        box->mesh->m_GraphicsPipeline = m_Adapter->Rdr()->CreateGraphicsPipeline(
            m_Adapter->RdrPass(),
            box->mesh->m_PipelineLayout,
            box->mesh->GetShaderName(),
            shaders,
            vertexInputInfo,
            VK_CULL_MODE_BACK_BIT,
            true, true, true, true,
            VK_POLYGON_MODE_LINE
        );

        for (uint32_t i = 0; i < box->mesh->m_UniformBuffers.size(); i++) {
            m_Adapter->Rdr()->UpdateUniformBuffer(
                box->mesh->m_UniformBuffers[i],
                box->mesh->GetData()->m_Ubos
            );
        }
    }

    VkDescriptorSetLayout Basic::CreateDescriptorSetLayout()
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

      VkDescriptorSetLayout desriptorSetLayout = m_Adapter->Rdr()->CreateDescriptorSetLayout(bindings);

      return desriptorSetLayout;
    }

    std::vector<VkDescriptorSet> Basic::CreateDescriptorSet(std::shared_ptr<Mesh> mesh, VkDescriptorSetLayout descriptorSetLayout)
    {
      if (!mesh->m_DescriptorSets.empty()) {
        vkFreeDescriptorSets(m_Adapter->Rdr()->GetDevice(), mesh->m_DescriptorPool, mesh->m_DescriptorSets.size(), mesh->m_DescriptorSets.data());
        mesh->m_DescriptorSets.clear();
      }

      std::vector<VkDescriptorImageInfo> imageInfos;

      Texture tex = m_TextureManager->GetTextures()[mesh->GetData()->m_Texture];

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = tex.GetImageView();
      imageInfo.sampler = tex.GetSampler();

      imageInfos.emplace_back(imageInfo);

      std::vector<VkDescriptorSet> descSets{};

      for (auto ubo : mesh->m_UniformBuffers) {
        VkDescriptorSet descSet = m_Adapter->Rdr()->CreateDescriptorSets(m_DescriptorPool, { descriptorSetLayout }, 1);
        m_Adapter->Rdr()->UpdateDescriptorSets({ ubo }, descSet, imageInfos);
        for (uint32_t i = 0; i < m_Adapter->GetSwapChainImages()->size(); i++) {
          descSets.emplace_back(descSet);
        }
      }

      return descSets;
    }

    VkPipelineLayout Basic::CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout)
    {
      std::vector<VkDescriptorSetLayout> dSetLayout = { descriptorSetLayout };

      std::vector<VkPushConstantRange> vkPcs = {};
      VkPushConstantRange vkPc;
      vkPc.offset = 0;
      vkPc.size = sizeof(constants);
      vkPc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
      vkPcs.emplace_back(vkPc);
      VkPipelineLayout pipelineLayout = m_Adapter->Rdr()->CreatePipelineLayout(dSetLayout, vkPcs);

      return pipelineLayout;
    }

    std::vector<VkPipelineShaderStageCreateInfo> Basic::GetShaders(std::string const & shaderName)
    {
      std::vector<VkPipelineShaderStageCreateInfo> shadersStageInfos;

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

      return shadersStageInfos;
    }

    void Basic::SetPushConstants(std::shared_ptr<Mesh> mesh)
    {
      constants pushConstants;
      pushConstants.data = glm::vec4(0.f, Poulpe::VulkanAdapter::s_AmbiantLight.load(), Poulpe::VulkanAdapter::s_FogDensity.load(), 0.f);
      pushConstants.cameraPos = m_Adapter->GetCamera()->GetPos();
      pushConstants.fogColor = glm::vec4({ Poulpe::VulkanAdapter::s_FogColor[0].load(), Poulpe::VulkanAdapter::s_FogColor[1].load(), Poulpe::VulkanAdapter::s_FogColor[2].load(), 0.f });
      pushConstants.lightPos = glm::vec4(m_Adapter->GetLights().at(0), 0.f);
      pushConstants.view = m_Adapter->GetCamera()->LookAt();
      pushConstants.ambiantLight = Poulpe::VulkanAdapter::s_AmbiantLight.load();
      pushConstants.fogDensity = Poulpe::VulkanAdapter::s_FogDensity.load();

      mesh->ApplyPushConstants = [=, &pushConstants](VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, std::shared_ptr<VulkanAdapter> adapter, const Data& data) {
        pushConstants.data = glm::vec4(static_cast<float>(data.m_TextureIndex), Poulpe::VulkanAdapter::s_AmbiantLight.load(), Poulpe::VulkanAdapter::s_FogDensity.load(), 0.f);
        pushConstants.cameraPos = adapter->GetCamera()->GetPos();
        pushConstants.fogColor = glm::vec4({ Poulpe::VulkanAdapter::s_FogColor[0].load(), Poulpe::VulkanAdapter::s_FogColor[1].load(), Poulpe::VulkanAdapter::s_FogColor[2].load(), 0.f });
        pushConstants.lightPos = glm::vec4(adapter->GetLights().at(0), 0.f);
        pushConstants.view = adapter->GetCamera()->LookAt();
        pushConstants.ambiantLight = Poulpe::VulkanAdapter::s_AmbiantLight.load();
        pushConstants.fogDensity = Poulpe::VulkanAdapter::s_FogDensity.load();
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants), &pushConstants);
      };

      mesh->SetHasPushConstants();
    }

    VkPipelineVertexInputStateCreateInfo Basic::GetVertexBindingDesc(VkVertexInputBindingDescription bDesc,
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
