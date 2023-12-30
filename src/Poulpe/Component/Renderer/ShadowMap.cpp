#include "ShadowMap.hpp"

namespace Poulpe
{
    struct constants;

    void ShadowMap::createDescriptorSet(IVisitable* const mesh)
    {
      std::vector<VkDescriptorImageInfo> imageInfos;
      std::vector<VkDescriptorImageInfo> imageInfoSpec;
      mesh->getData()->mapsUsed = glm::vec4(0.0f);

      Texture tex;
      tex = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(0)];

      if (tex.getName() == "_plp_empty") {
        mesh->getData()->mapsUsed.w = 1.0f;
      }

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = tex.getImageView();
      imageInfo.sampler = tex.getSampler();

      imageInfos.emplace_back(imageInfo);

      auto pipeline = m_Renderer->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet = m_Renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {

        m_Renderer->updateDescriptorSets(
          *mesh->getUniformBuffers(),
          *mesh->getStorageBuffers(),
          descSet, imageInfos);
      }

      mesh->setDescSet(descSet);
    }

    void ShadowMap::setPushConstants(IVisitable* const mesh)
    {
        mesh->setApplyPushConstants([](
            VkCommandBuffer & commandBuffer, 
            VkPipelineLayout pipelineLayout,
            IRenderer* const renderer, IVisitable* const mesh) {

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(mesh->getData()->m_TextureIndex, 0.0, 0.0);
            pushConstants.view = renderer->getCamera()->lookAt();
            pushConstants.viewPos = renderer->getCamera()->getPos();
            pushConstants.mapsUsed = mesh->getData()->mapsUsed;

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
                & pushConstants);
        });

        mesh->setHasPushConstants();
    }

    void ShadowMap::visit([[maybe_unused]] float const deltaTime, IVisitable* const mesh)
    {
      if (!mesh && !mesh->isDirty()) return;

      uint32_t totalInstances = static_cast<uint32_t>(mesh->getData()->m_Ubos.size());
      uint32_t maxUniformBufferRange = m_Renderer->getDeviceProperties().limits.maxUniformBufferRange;
      uint32_t uniformBufferChunkSize = maxUniformBufferRange / sizeof(UniformBufferObject);
      uint32_t uniformBuffersCount = static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize)));

      //@todo fix memory management...
      uint32_t uboOffset = (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances;
      uint32_t uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
      uint32_t nbUbo = uboOffset;

      for (size_t i = 0; i < uniformBuffersCount; ++i) {

        mesh->getData()->m_UbosOffset.emplace_back(uboOffset);
        Buffer uniformBuffer = m_Renderer->createUniformBuffers(nbUbo);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);

        uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
        nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
        uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
      }

      auto commandPool = m_Renderer->createCommandPool();
      auto data = mesh->getData();

      data->m_VertexBuffer = m_Renderer->createVertexBuffer(commandPool, data->m_Vertices);
      data->m_IndicesBuffer = m_Renderer->createIndexBuffer(commandPool, data->m_Indices);
      data->m_TextureIndex = 0;

      for (size_t i = 0; i < mesh->getData()->m_Ubos.size(); ++i) {
        mesh->getData()->m_Ubos[i].projection = m_Renderer->getPerspective();

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
      auto storageBuffer = m_Renderer->createStorageBuffers(size);
      mesh->addStorageBuffer(storageBuffer);
      m_Renderer->updateStorageBuffer(mesh->getStorageBuffers()->at(0), objectBuffer);
      mesh->setHasBufferStorage();

      unsigned int min{ 0 };
      unsigned int max{ 0 };

      for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {
        max = mesh->getData()->m_UbosOffset.at(i);
        auto ubos = std::vector<UniformBufferObject>(mesh->getData()->m_Ubos.begin() + min, mesh->getData()->m_Ubos.begin() + max);

        m_Renderer->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

        min = max;
      }

      createDescriptorSet(mesh);
      setPushConstants(mesh);
      mesh->setIsDirty(false);
    }
}
