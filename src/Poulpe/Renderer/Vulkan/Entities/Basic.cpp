#include "Basic.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct constants;

    Basic::Basic(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager)
        : m_Adapter(adapter),
        m_ShaderManager(shaderManager),
        m_TextureManager(textureManager)
    {
      std::vector<VkDescriptorPoolSize> poolSizes{};
      VkDescriptorPoolSize cp1;
      cp1.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      cp1.descriptorCount = 10;

      VkDescriptorPoolSize cp2;
      cp2.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      cp2.descriptorCount = 10;
      poolSizes.emplace_back(cp1);
      poolSizes.emplace_back(cp2);

      VkDescriptorPoolSize cp3;
      cp3.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      cp3.descriptorCount = 10;

      poolSizes.emplace_back(cp1);
      poolSizes.emplace_back(cp2);
      poolSizes.emplace_back(cp3);


      m_DescriptorPool = m_Adapter->rdr()->createDescriptorPool(poolSizes, 1000);
    }

    void Basic::visit([[maybe_unused]] float const deltaTime, Mesh* mesh)
    {
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
          Mesh::Buffer uniformBuffer = m_Adapter->rdr()->createUniformBuffers(nbUbo);
          mesh->getUniformBuffers()->emplace_back(uniformBuffer);

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
          mesh->getData()->m_Ubos[i].projection = m_Adapter->getPerspective();
        }

        mesh->setDescriptorSetLayout(createDescriptorSetLayout());
        mesh->setDescriptorSets(createDescriptorSet(mesh));
        mesh->setPipelineLayout(createPipelineLayout(mesh->getDescriptorSetLayout()));

        m_Adapter->getDescriptorSetLayouts()->emplace_back(mesh->getDescriptorSetLayout());

        int min{ 0 };
        int max{ 0 };

        for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {
          max = mesh->getData()->m_UbosOffset.at(i);
          auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);

          m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

          min = max;
        }

        setPushConstants(mesh);

        auto shaders = getShaders(mesh->getShaderName());
        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attDesc);

        mesh->setGraphicsPipeline(m_Adapter->rdr()->createGraphicsPipeline(m_Adapter->rdrPass(), mesh->getPipelineLayout(),
            mesh->getShaderName(), shaders, vertexInputInfo, VK_CULL_MODE_BACK_BIT, true, true, true, true,
            VulkanAdapter::s_PolygoneMode));

        if (m_Adapter->getDrawBbox() && mesh->hasBbox()) {
            createBBoxEntity(mesh);
            mesh->setHasBbox(true);
        }

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        //loading normal map
        if (!mesh->getData()->m_TextureBumpMap.empty()) {
            auto const & map = m_TextureManager->addNormalMapTexture(mesh->getData()->m_TextureBumpMap);
            
            if (map.size() > 0) {
              size_t size = sizeof(std::array<float, 3>) * map.size();
              mesh->addStorageBuffer(m_Adapter->rdr()->createStorageBuffers(size));
            }
        }

        mesh->setIsDirty(false);
    }

    void Basic::createBBoxEntity(Mesh* mesh)
    {
        Mesh::BBox* box = mesh->getBBox();

        UniformBufferObject ubo{};
        glm::mat4 transform = glm::translate(glm::mat4(1), box->center) * glm::scale(glm::mat4(1), box->size);
        ubo.model = box->position * transform;

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        const std::vector<Vertex> vertices = {
            {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0, //front
            1, 5, 6, 6, 2, 1, // right
            7, 6, 5, 5, 4, 7, // back
            4, 0, 3, 3, 7, 4, // left
            4, 5, 1, 1, 0, 4, // bottom
            3, 2, 6, 6, 7, 3 // top
        };

        Mesh::Data data;
        data.m_Texture = "mpoulpe";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        data.m_Indices = indices;
        data.m_Vertices = vertices;
        data.m_Ubos.emplace_back(ubo);

        if (box->mesh->getData()->m_Ubos.size() > 0)
            data.m_Ubos.insert(data.m_Ubos.end(), box->mesh->getData()->m_Ubos.begin(), box->mesh->getData()->m_Ubos.end());

        for (uint32_t i = 0; i < data.m_Ubos.size(); i++) {
            data.m_Ubos[i].projection = m_Adapter->getPerspective();
        }

        box->mesh->setName("bbox_" + mesh->getData()->m_Name);
        box->mesh->setShaderName("bbox");
        box->mesh->setData(data);
        box->mesh->setDescriptorSetLayout(createDescriptorSetLayout());
        box->mesh->setDescriptorSets(createDescriptorSet(box->mesh.get()));
        box->mesh->setPipelineLayout(createPipelineLayout(box->mesh->getDescriptorSetLayout()));

        setPushConstants(box->mesh.get());

        auto shaders = Basic::getShaders("bbox");
        auto bDesc = Vertex::GetBindingDescription();
        auto attDesc = Vertex::GetAttributeDescriptions();
        auto vertexInputInfo = getVertexBindingDesc(bDesc, attDesc);

        box->mesh->setGraphicsPipeline(m_Adapter->rdr()->createGraphicsPipeline(m_Adapter->rdrPass(),
            box->mesh->getPipelineLayout(), box->mesh->getShaderName(), shaders, vertexInputInfo,
            VK_CULL_MODE_BACK_BIT, true, true, true, true, VK_POLYGON_MODE_LINE));

        for (uint32_t i = 0; i < box->mesh->getUniformBuffers()->size(); i++) {
            m_Adapter->rdr()->updateUniformBuffer(
                box->mesh->getUniformBuffers()->at(i),
                & box->mesh->getData()->m_Ubos
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
      samplerLayoutBinding.descriptorCount = 3;
      samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      samplerLayoutBinding.pImmutableSamplers = nullptr;
      samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      VkDescriptorSetLayoutBinding normalMap{};
      normalMap.binding = 2;
      normalMap.descriptorCount = 1;
      normalMap.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      normalMap.pImmutableSamplers = nullptr;
      normalMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      std::vector<VkDescriptorSetLayoutBinding> bindings = { 
          uboLayoutBinding, samplerLayoutBinding, normalMap };

      VkDescriptorSetLayout desriptorSetLayout = m_Adapter->rdr()->createDescriptorSetLayout(bindings);

      return desriptorSetLayout;
    }

    std::vector<VkDescriptorSet> Basic::createDescriptorSet(Mesh* mesh)
    {
      if (!mesh->getDescriptorSets().empty()) {
        vkFreeDescriptorSets(m_Adapter->rdr()->getDevice(), mesh->getDescriptorPool(),
            mesh->getDescriptorSets().size(), mesh->getDescriptorSets().data());

        mesh->getDescriptorSets().clear();
      }

      std::vector<VkDescriptorImageInfo> imageInfos;
      std::vector<VkDescriptorImageInfo> imageInfoSpec;

      Texture tex;

      if (m_TextureManager->getTextures().contains(mesh->getData()->m_Texture)) {
        tex = m_TextureManager->getTextures()[mesh->getData()->m_Texture];
      } else {
        tex = m_TextureManager->getTextures()["mpoulpe"];
      }

      VkDescriptorImageInfo imageInfo{};

      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = tex.getImageView();
      imageInfo.sampler = tex.getSampler();

      imageInfos.emplace_back(imageInfo);

      std::string specMapName = "mpoulpe";
      std::string bumMapName = "mpoulpe";

      if (!mesh->getData()->m_TextureSpecularMap.empty() 
          && m_TextureManager->getTextures().contains(mesh->getData()->m_TextureSpecularMap)) {
          specMapName = mesh->getData()->m_TextureSpecularMap;
          mesh->getData()->mapsUsed.y = 1.0f;

      }
      Texture texSpecularMap = m_TextureManager->getTextures()[specMapName];

      if (!mesh->getData()->m_TextureBumpMap.empty() 
          && m_TextureManager->getTextures().contains(mesh->getData()->m_TextureBumpMap)) {
          bumMapName = mesh->getData()->m_TextureBumpMap;
        mesh->getData()->mapsUsed.x = 1.0f;
      }

      Texture texBumpMap = m_TextureManager->getTextures()[bumMapName];

      VkDescriptorImageInfo imageInfoSpecularMap{};
      imageInfoSpecularMap.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfoSpecularMap.imageView = texSpecularMap.getImageView();
      imageInfoSpecularMap.sampler = texSpecularMap.getSampler();

      VkDescriptorImageInfo imageInfoBumpMap{};
      imageInfoBumpMap.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfoBumpMap.imageView = texBumpMap.getImageView();
      imageInfoBumpMap.sampler = texBumpMap.getSampler();

      imageInfos.emplace_back(imageInfoSpecularMap);
      imageInfos.emplace_back(imageInfoBumpMap);
 
      std::vector<VkDescriptorSet> descSets{};

      for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {
        VkDescriptorSet descSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { mesh->getDescriptorSetLayout()}, 1);

        m_Adapter->rdr()->updateDescriptorSet(mesh->getUniformBuffers()->at(i), descSet, imageInfos);

        for (uint32_t i = 0; i < m_Adapter->getSwapChainImages()->size(); i++) {
          descSets.emplace_back(descSet);
        }
      }

      for (size_t i = 0; i < mesh->getStorageBuffers()->size(); ++i) {
          VkDescriptorSet descSet = m_Adapter->rdr()->createDescriptorSets(m_DescriptorPool, { mesh->getDescriptorSetLayout() }, 1);

          m_Adapter->rdr()->updateStorageDescriptorSet(mesh->getStorageBuffers()->at(i), descSet);

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

    void Basic::setPushConstants(Mesh* mesh)
    {
        mesh->applyPushConstants = [](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* adapter, Mesh* mesh) {

            constants pushConstants{};
            pushConstants.textureID = mesh->getData()->m_TextureIndex;
            
            
            pushConstants.view = adapter->getCamera()->lookAt();
            pushConstants.viewPos = adapter->getCamera()->getPos();
            
            pushConstants.lightDir = glm::vec3(0.5, 2.5, -0.2);

            pushConstants.ambient = mesh->getMaterial().ambient;
            pushConstants.ambientLight = glm::vec3(1.0);
            pushConstants.ambientLightColor = glm::vec3(1.0f);

            pushConstants.diffuseLight = glm::vec3(0.8);

            pushConstants.specular = mesh->getMaterial().specular;
            pushConstants.specularLight = glm::vec3(1.0);

            pushConstants.shininess = mesh->getMaterial().shininess;

            pushConstants.mapsUsed = mesh->getData()->mapsUsed;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(constants),
                & pushConstants);
        };

        mesh->setHasPushConstants();
    }

    VkPipelineVertexInputStateCreateInfo Basic::getVertexBindingDesc(VkVertexInputBindingDescription bDesc,
      std::array<VkVertexInputAttributeDescription, 4> attDesc)
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
