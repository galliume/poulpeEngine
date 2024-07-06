#include "Basic.hpp"

namespace Poulpe
{
    struct constants;

    void Basic::createDescriptorSet(IVisitable* const mesh)
    {
      std::vector<VkDescriptorImageInfo> imageInfos;
      std::vector<VkDescriptorImageInfo> imageInfoSpec;

      Texture tex;
      tex = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(0)];

      Texture tex2;
      tex2 = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(1)];

      Texture tex3;
      tex3 = m_TextureManager->getTextures()[mesh->getData()->m_Textures.at(2)];

      Texture alpha;
      alpha = m_TextureManager->getTextures()[mesh->getData()->m_TextureAlpha];

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
      }

      Texture texSpecularMap = m_TextureManager->getTextures()[specMapName];

      if (!mesh->getData()->m_TextureBumpMap.empty()
        && m_TextureManager->getTextures().contains(mesh->getData()->m_TextureBumpMap)) {
        bumpMapName = mesh->getData()->m_TextureBumpMap;
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
      shadowMapAmbient.imageView = m_Renderer->getDepthMapImageViews()->at(0);
      shadowMapAmbient.sampler = m_Renderer->getDepthMapSamplers()->at(0);

      //VkDescriptorImageInfo shadowMapSpot{};
      //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //shadowMapSpot.imageView = m_Renderer->getDepthMapImageViews()->at(1);
      //shadowMapSpot.sampler = m_Renderer->getDepthMapSamplers()->at(1);

      imageInfos.emplace_back(imageInfoSpecularMap);
      imageInfos.emplace_back(imageInfoBumpMap);
      imageInfos.emplace_back(shadowMapAmbient);
      //imageInfos.emplace_back(shadowMapSpot);

      auto pipeline = m_Renderer->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet = m_Renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      for (size_t i = 0; i < mesh->getUniformBuffers()->size(); ++i) {

        m_Renderer->updateDescriptorSets(
          *mesh->getUniformBuffers(),
          *mesh->getStorageBuffers(),
          descSet, imageInfos);
      }

      mesh->setDescSet(descSet);

      std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
      std::vector<VkDescriptorBufferInfo> bufferInfos;
      std::vector<VkDescriptorBufferInfo> storageBufferInfos;

      std::for_each(std::begin(*mesh->getUniformBuffers()), std::end(*mesh->getUniformBuffers()),
        [&bufferInfos](const Buffer& uniformBuffer)
        {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = uniformBuffer.buffer;
          bufferInfo.offset = 0;
          bufferInfo.range = VK_WHOLE_SIZE;
          bufferInfos.emplace_back(bufferInfo);
        });

     auto shadowMapPipeline = m_Renderer->getPipeline("shadowMap");
     VkDescriptorSet shadowMapDescSet = m_Renderer->createDescriptorSets(shadowMapPipeline->descPool, { shadowMapPipeline->descSetLayout }, 1);

      std::for_each(std::begin(*mesh->getStorageBuffers()), std::end(*mesh->getStorageBuffers()),
        [&storageBufferInfos](const Buffer& storageBuffers)
        {
          VkDescriptorBufferInfo bufferInfo{};
          bufferInfo.buffer = storageBuffers.buffer;
          bufferInfo.offset = 0;
          bufferInfo.range = VK_WHOLE_SIZE;
          storageBufferInfos.emplace_back(bufferInfo);
        });

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = shadowMapDescSet;
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = bufferInfos.data();

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = shadowMapDescSet;
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorWrites[1].descriptorCount = static_cast<uint32_t>(storageBufferInfos.size());
      descriptorWrites[1].pBufferInfo = storageBufferInfos.data();

      vkUpdateDescriptorSets(m_Renderer->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

      mesh->setShadowMapDescSet(shadowMapDescSet);
    }

    void Basic::setPushConstants(IVisitable* const mesh)
    {
        mesh->setApplyPushConstants([](
            VkCommandBuffer & commandBuffer,
            VkPipelineLayout pipelineLayout,
            IRenderer* const renderer, IVisitable* const meshB) {

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(meshB->getData()->m_TextureIndex, 0.0, 0.0);
            pushConstants.view = renderer->getCamera()->lookAt();
            pushConstants.viewPos = renderer->getCamera()->getPos();

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
                & pushConstants);
        });

        mesh->setHasPushConstants();
    }

    void Basic::visit([[maybe_unused]] std::chrono::duration<float> deltaTime, IVisitable* const mesh)
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
