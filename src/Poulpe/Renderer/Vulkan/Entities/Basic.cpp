#include "Basic.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct constants;

    Basic::Basic(VulkanAdapter* adapter,
        TextureManager* textureManager,
        LightManager* lightManager)
        : m_Adapter(adapter),
        m_TextureManager(textureManager),
        m_LightManager(lightManager)
    {

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

          if (m_TextureManager->getTextures().contains(mesh->getData()->m_TextureBumpMap)) {
              auto const tex = m_TextureManager->getTextures()[mesh->getData()->m_TextureBumpMap];
              mesh->getData()->m_Ubos[i].texSize = glm::vec2(tex.getWidth(), tex.getHeight());
          }
        }

        ObjectBuffer objectBuffer{};

        Material material{};
        material.ambient = mesh->getMaterial().ambient;
        material.diffuse = mesh->getMaterial().diffuse;
        material.specular = mesh->getMaterial().specular;
        material.transmittance = mesh->getMaterial().transmittance;
        material.emission = mesh->getMaterial().emission;
        material.shiIorDiss = glm::vec3(mesh->getMaterial().shininess,
            mesh->getMaterial().ior, mesh->getMaterial().illum);

        objectBuffer.pointLights[0] = m_LightManager->getPointLights().at(0);
        objectBuffer.pointLights[1] = m_LightManager->getPointLights().at(1);

        objectBuffer.spotLight = m_LightManager->getSpotLights().at(0);

        objectBuffer.ambientLight = m_LightManager->getAmbientLight();

        objectBuffer.material = material;

        auto size = sizeof(objectBuffer);
        mesh->addStorageBuffer(m_Adapter->rdr()->createStorageBuffers(size));
        m_Adapter->rdr()->updateStorageBuffer(mesh->getStorageBuffers()->at(0), objectBuffer);
        mesh->setHasBufferStorage();

        int min{ 0 };
        int max{ 0 };

        for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {
          max = mesh->getData()->m_UbosOffset.at(i);
          auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);

          m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

          min = max;
        }

        createDescriptorSet(mesh);
        setPushConstants(mesh);
        mesh->setIsDirty(false);
    }

    void Basic::setPushConstants(Mesh* mesh)
    {
        mesh->applyPushConstants = [](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* adapter, Mesh* mesh) {

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(mesh->getData()->m_TextureIndex, 0.0, 0.0);
            pushConstants.view = adapter->getCamera()->lookAt();
            pushConstants.viewPos = adapter->getCamera()->getPos();
            pushConstants.mapsUsed = mesh->getData()->mapsUsed;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
                & pushConstants);
        };

        mesh->setHasPushConstants();
    }

    void Basic::createDescriptorSet(Mesh* mesh)
    {
      std::vector<VkDescriptorImageInfo> imageInfos;
      std::vector<VkDescriptorImageInfo> imageInfoSpec;
      mesh->getData()->mapsUsed = glm::vec4(0.0f);

      Texture tex;
      tex = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(0)];

      if (tex.getName() == "_plp_empty") {
          mesh->getData()->mapsUsed.w = 1.0f;
      }

      Texture tex2;
      tex2 = m_TextureManager->getTextures()["_plp_empty"];

      Texture tex3;
      tex3 = m_TextureManager->getTextures()["_plp_empty"];

      Texture alpha;
      alpha = m_TextureManager->getTextures()["_plp_empty"];

      if (!mesh->getData()->m_Textures.at(1).empty()) {
        tex2 = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(1)];
      }

      if (!mesh->getData()->m_Textures.at(2).empty()) {
        tex3 = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(2)];
      }

      if (!mesh->getData()->m_TextureAlpha.empty()) {
        mesh->getData()->mapsUsed.z = 1.0f;
        alpha = m_TextureManager->getTextures()[mesh->getData()->m_TextureAlpha];
      }

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = tex.getImageView();
      imageInfo.sampler = tex.getSampler();

      VkDescriptorImageInfo imageInfo2{};
      imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo2.imageView = tex2.getImageView();
      imageInfo2.sampler = tex2.getSampler();

      VkDescriptorImageInfo imageInfo3{};
      imageInfo3.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo3.imageView = tex3.getImageView();
      imageInfo3.sampler = tex3.getSampler();

      VkDescriptorImageInfo imageInfoAlpha{};
      imageInfoAlpha.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfoAlpha.imageView = alpha.getImageView();
      imageInfoAlpha.sampler = alpha.getSampler();

      imageInfos.emplace_back(imageInfo);
      imageInfos.emplace_back(imageInfo2);
      imageInfos.emplace_back(imageInfo3);
      imageInfos.emplace_back(imageInfoAlpha);

      std::string specMapName = "_plp_empty";
      std::string bumpMapName = "_plp_empty";

      if (!mesh->getData()->m_TextureSpecularMap.empty()
        && m_TextureManager->getTextures().contains(mesh->getData()->m_TextureSpecularMap)) {
        specMapName = mesh->getData()->m_TextureSpecularMap;
        mesh->getData()->mapsUsed.y = 1.0f;
      }

      Texture texSpecularMap = m_TextureManager->getTextures()[specMapName];

      if (!mesh->getData()->m_TextureBumpMap.empty()
        && m_TextureManager->getTextures().contains(mesh->getData()->m_TextureBumpMap)) {
        bumpMapName = mesh->getData()->m_TextureBumpMap;
        mesh->getData()->mapsUsed.x = 1.0f;
      }

      Texture texBumpMap = m_TextureManager->getTextures()[bumpMapName];

      VkDescriptorImageInfo imageInfoSpecularMap{};
      imageInfoSpecularMap.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfoSpecularMap.imageView = texSpecularMap.getImageView();
      imageInfoSpecularMap.sampler = texSpecularMap.getSampler();

      VkDescriptorImageInfo imageInfoBumpMap{};
      imageInfoBumpMap.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfoBumpMap.imageView = texBumpMap.getImageView();
      imageInfoBumpMap.sampler = texBumpMap.getSampler();

      VkDescriptorImageInfo shadowMapAmbient{};
      shadowMapAmbient.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      shadowMapAmbient.imageView = m_Adapter->getDepthMapImageViews()->at(0);
      shadowMapAmbient.sampler = m_Adapter->getDepthMapSamplers()->at(0);

      //VkDescriptorImageInfo shadowMapSpot{};
      //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //shadowMapSpot.imageView = m_Adapter->getDepthMapImageViews()->at(1);
      //shadowMapSpot.sampler = m_Adapter->getDepthMapSamplers()->at(1);

      imageInfos.emplace_back(imageInfoSpecularMap);
      imageInfos.emplace_back(imageInfoBumpMap);
      imageInfos.emplace_back(shadowMapAmbient);
      //imageInfos.emplace_back(shadowMapSpot);

      auto pipeline = m_Adapter->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet = m_Adapter->rdr()->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {

        m_Adapter->rdr()->updateDescriptorSets(
          *mesh->getUniformBuffers(),
          *mesh->getStorageBuffers(),
          descSet, imageInfos);
      }

      mesh->setDescSet(descSet);
    }
}
