#include "Basic.hpp"

namespace Poulpe
{
    struct constants;

    void Basic::createDescriptorSet(Mesh* mesh)
    {
      Texture const tex{ _texture_manager->getTextures()[mesh->getData()->_textures.at(0)] };
      Texture const tex2{ _texture_manager->getTextures()[mesh->getData()->_textures.at(1)] };
      Texture const tex3{ _texture_manager->getTextures()[mesh->getData()->_textures.at(2)] };
      Texture const alpha{ _texture_manager->getTextures()[mesh->getData()->_alpha] };

      std::vector<VkDescriptorImageInfo> imageInfos{};
      imageInfos.reserve(8);
      imageInfos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(tex2.getSampler(), tex2.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(tex3.getSampler(), tex3.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(alpha.getSampler(), alpha.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      std::string specMapName{ "_plp_empty" };
      std::string bumpMapName{ "_plp_empty" };

      if (!mesh->getData()->_specular_map.empty()
        && _texture_manager->getTextures().contains(mesh->getData()->_specular_map)) {
        specMapName = mesh->getData()->_specular_map;
      }

      Texture const texSpecularMap{ _texture_manager->getTextures()[specMapName] };

      if (!mesh->getData()->_bump_map.empty()
        && _texture_manager->getTextures().contains(mesh->getData()->_bump_map)) {
        bumpMapName = mesh->getData()->_bump_map;
      }

      Texture const texBumpMap{ _texture_manager->getTextures()[bumpMapName] };

      //VkDescriptorImageInfo shadowMapSpot{};
      //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //shadowMapSpot.imageview = _renderer->getDepthMapImageViews()->at(1);
      //shadowMapSpot.sampler = _renderer->getDepthMapSamplers()->at(1);
      imageInfos.emplace_back(texSpecularMap.getSampler(), texSpecularMap.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(texBumpMap.getSampler(), texBumpMap.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(_renderer->getDepthMapSamplers()->at(0), _renderer->getDepthMapImageViews()->at(0), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      //imageInfos.emplace_back(shadowMapSpot);

      auto const& pipeline = _renderer->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet{ _renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1) };

      for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {

        _renderer->updateDescriptorSets(
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

     auto const& shadowMapPipeline = _renderer->getPipeline("shadowMap");
     VkDescriptorSet shadowMapDescSet = _renderer->createDescriptorSets(shadowMapPipeline->descPool, { shadowMapPipeline->descSetLayout }, 1);

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

      vkUpdateDescriptorSets(_renderer->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

      mesh->setShadowMapDescSet(shadowMapDescSet);
    }

    void Basic::setPushConstants(Mesh* mesh)
    {
        mesh->setApplyPushConstants([](
            VkCommandBuffer & cmd_buffer,
            VkPipelineLayout pipelineLayout,
            Renderer* const renderer, Mesh* const meshB) {

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(meshB->getData()->_texture_index, 0.0, 0.0);
            pushConstants.view = renderer->getCamera()->lookAt();
            pushConstants.viewPos = renderer->getCamera()->getPos();

            vkCmdPushConstants(cmd_buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
                & pushConstants);
        });

        mesh->setHasPushConstants();
    }

    void Basic::operator()(std::chrono::duration<float> const& deltaTime, Mesh* mesh)
    {
      if (!mesh && !mesh->isDirty()) return;

      if (mesh->getData()->_ubos_offset.empty()) {
        uint32_t const totalInstances{ static_cast<uint32_t>(mesh->getData()->_ubos.size()) };
        uint32_t const maxUniformBufferRange{ _renderer->getDeviceProperties().limits.maxUniformBufferRange };
        unsigned long long const uniformBufferChunkSize{ maxUniformBufferRange / sizeof(UniformBufferObject) };
        uint32_t const uniformBuffersCount{ static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize))) };

        //@todo fix memory management...
        unsigned long long uboOffset{ (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances };
        unsigned long long uboRemaining{ (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0 };
        unsigned long long nbUbo{ uboOffset };

        for (size_t i{ 0 }; i < uniformBuffersCount; ++i) {

          mesh->getData()->_ubos_offset.emplace_back(uboOffset);
          Buffer uniformBuffer = _renderer->createUniformBuffers(nbUbo);
          mesh->getUniformBuffers()->emplace_back(uniformBuffer);

          uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
          nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
          uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
        }
      }

      auto const& data = mesh->getData();
      data->_texture_index = 0;

      if (data->_vertex_buffer.buffer == VK_NULL_HANDLE) {
        auto commandPool = _renderer->createCommandPool();
        data->_vertex_buffer = _renderer->createVertexBuffer(commandPool, data->_vertices);
        data->_indices_buffer = _renderer->createIndexBuffer(commandPool, data->_Indices);
      } else {
        //suppose we have to update data
        {
          data->_vertex_buffer.memory->lock();

          void* newData;
          vkMapMemory(_renderer->getDevice(), *data->_vertex_buffer.memory->getMemory(), data->_vertex_buffer.offset, data->_vertex_buffer.size, 0, &newData);
          memcpy(newData, data->_vertices.data(), data->_vertex_buffer.size);
          vkUnmapMemory(_renderer->getDevice(), *data->_vertex_buffer.memory->getMemory());

          data->_vertex_buffer.memory->unLock();
        }
        {
          data->_indices_buffer.memory->lock();

          void* newData;
          vkMapMemory(_renderer->getDevice(), *data->_indices_buffer.memory->getMemory(), data->_indices_buffer.offset, data->_indices_buffer.size, 0, &newData);
          memcpy(newData, data->_Indices.data(), data->_indices_buffer.size);
          vkUnmapMemory(_renderer->getDevice(), *data->_indices_buffer.memory->getMemory());

          data->_indices_buffer.memory->unLock();
        }
      }

      for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); ++i) {
        mesh->getData()->_ubos[i].projection = _renderer->getPerspective();

        if (_texture_manager->getTextures().contains(mesh->getData()->_bump_map)) {
          auto const tex = _texture_manager->getTextures()[mesh->getData()->_bump_map];
          mesh->getData()->_ubos[i].texSize = glm::vec2(tex.getWidth(), tex.getHeight());
        }
      }

      if (mesh->getStorageBuffers()->empty()) {

        Material material{};
        material.ambient = mesh->getMaterial().ambient;
        material.diffuse = mesh->getMaterial().diffuse;
        material.specular = mesh->getMaterial().specular;
        material.transmittance = mesh->getMaterial().transmittance;
        material.emission = mesh->getMaterial().emission;
        material.shiIorDiss = glm::vec3(mesh->getMaterial().shininess,
          mesh->getMaterial().ior, mesh->getMaterial().illum);

        ObjectBuffer objectBuffer{};
        objectBuffer.pointLights[0] = _light_manager->getPointLights().at(0);
        objectBuffer.pointLights[1] = _light_manager->getPointLights().at(1);
        objectBuffer.spotLight = _light_manager->getSpotLights().at(0);
        objectBuffer.ambientLight = _light_manager->getAmbientLight();
        objectBuffer.material = material;

        auto const size{ sizeof(objectBuffer) };
        auto storageBuffer{ _renderer->createStorageBuffers(size) };

        mesh->setObjectBuffer(objectBuffer);
        mesh->addStorageBuffer(storageBuffer);
        mesh->setHasBufferStorage();

        _renderer->updateStorageBuffer(mesh->getStorageBuffers()->at(0), objectBuffer);
      }

      unsigned int min{ 0 };
      unsigned int max{ 0 };

      for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
        max = mesh->getData()->_ubos_offset.at(i);
        auto ubos = std::vector<UniformBufferObject>(mesh->getData()->_ubos.begin() + min, mesh->getData()->_ubos.begin() + max);

        min = max;
        if (ubos.empty()) continue;
        _renderer->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);
      }

      if (*mesh->getDescSet() == NULL) {

        createDescriptorSet(mesh);
        setPushConstants(mesh);
      }
      mesh->setIsDirty(false);
    }
}
