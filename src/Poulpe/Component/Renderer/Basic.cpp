#include "Basic.hpp"

namespace Poulpe
{
  struct constants;

  void Basic::createDescriptorSet(Mesh* mesh)
  {
    Texture const tex{ _texture_manager->getTextures()[mesh->getData()->_textures.at(0)] };
    Texture const alpha{ _texture_manager->getTextures()[mesh->getData()->_alpha] };

    std::vector<VkDescriptorImageInfo> image_info{};
    image_info.reserve(5);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(alpha.getSampler(), alpha.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::string const bump_map_name{ mesh->getData()->_bump_map };
    Texture texture_bump{ _texture_manager->getTextures()[bump_map_name] };
    
    if (texture_bump.getWidth() == 0) {
      texture_bump = _texture_manager->getTextures()["_plp_empty"];
    }

    std::string const specular_map_name{ mesh->getData()->_specular_map };
    Texture texture_specular{ _texture_manager->getTextures()[specular_map_name] };

    if (texture_specular.getWidth() == 0) {
      texture_specular = _texture_manager->getTextures()["_plp_empty"];
    }
    //VkDescriptorImageInfo shadowMapSpot{};
    //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //shadowMapSpot.imageview = _renderer->getDepthMapImageViews()->at(1);
    //shadowMapSpot.sampler = _renderer->getDepthMapSamplers()->at(1);
    image_info.emplace_back(texture_specular.getSampler(), texture_specular.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(texture_bump.getSampler(), texture_bump.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(_renderer->getDepthMapSamplers(), _renderer->getDepthMapImageViews(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //image_info.emplace_back(shadowMapSpot);

    auto const& pipeline = _renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset{ _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1) };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {

      _renderer->getAPI()->updateDescriptorSets(
        *mesh->getUniformBuffers(),
        *mesh->getStorageBuffers(),
        descset, image_info);
    }

    mesh->setDescSet(descset);

    std::array<VkWriteDescriptorSet, 2> descset_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorBufferInfo> buffer_storage_infos;

    std::for_each(std::begin(*mesh->getUniformBuffers()), std::end(*mesh->getUniformBuffers()),
      [&buffer_infos](const Buffer& uniformBuffer)
      {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniformBuffer.buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        buffer_infos.emplace_back(buffer_info);
      });

    auto const& shadow_map_pipeline = _renderer->getPipeline("shadowMap");
    VkDescriptorSet shadow_map_descset = _renderer->getAPI()->createDescriptorSets(shadow_map_pipeline->desc_pool, { shadow_map_pipeline->descset_layout }, 1);

    std::for_each(std::begin(*mesh->getStorageBuffers()), std::end(*mesh->getStorageBuffers()),
      [&buffer_storage_infos](const Buffer& storageBuffers)
      {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = storageBuffers.buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        buffer_storage_infos.emplace_back(buffer_info);
      });

    descset_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descset_writes[0].dstSet = shadow_map_descset;
    descset_writes[0].dstBinding = 0;
    descset_writes[0].dstArrayElement = 0;
    descset_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descset_writes[0].descriptorCount = 1;
    descset_writes[0].pBufferInfo = buffer_infos.data();

    descset_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descset_writes[1].dstSet = shadow_map_descset;
    descset_writes[1].dstBinding = 1;
    descset_writes[1].dstArrayElement = 0;
    descset_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descset_writes[1].descriptorCount = static_cast<uint32_t>(buffer_storage_infos.size());
    descset_writes[1].pBufferInfo = buffer_storage_infos.data();

    vkUpdateDescriptorSets(_renderer->getDevice(), static_cast<uint32_t>(descset_writes.size()), descset_writes.data(), 0, nullptr);

    mesh->setShadowMapDescSet(shadow_map_descset);
  }

  void Basic::setPushConstants(Mesh* mesh)
  {
    mesh->setApplyPushConstants([](
      VkCommandBuffer & cmd_buffer,
      VkPipelineLayout pipeline_layout,
      Renderer* const renderer, Mesh* const meshB) {

      constants pushConstants{};
      pushConstants.view = renderer->getCamera()->lookAt();
      pushConstants.view_position = renderer->getCamera()->getPos();

      vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
        &pushConstants);
    });

    mesh->setHasPushConstants();
  }

  void Basic::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;

    auto commandPool = _renderer->getAPI()->createCommandPool();

    if (mesh->getData()->_ubos_offset.empty()) {
      uint32_t const totalInstances{ static_cast<uint32_t>(mesh->getData()->_ubos.size()) };
      uint32_t const maxUniformBufferRange{ _renderer->getAPI()->getDeviceProperties().limits.maxUniformBufferRange };
      unsigned long long const uniformBufferChunkSize{ maxUniformBufferRange / sizeof(UniformBufferObject) };
      uint32_t const uniformBuffersCount{ static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize))) };

      //@todo fix memory management...
      unsigned long long uboOffset{ (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances };
      unsigned long long uboRemaining{ (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0 };
      unsigned long long nbUbo{ uboOffset };

      for (size_t i{ 0 }; i < uniformBuffersCount; ++i) {

        mesh->getData()->_ubos_offset.emplace_back(uboOffset);
        Buffer uniformBuffer = _renderer->getAPI()->createUniformBuffers(nbUbo, commandPool);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);

        uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
        nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
        uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
      }
    }

    auto const& data = mesh->getData();
    data->_texture_index = 0;

    if (data->_vertex_buffer.buffer == VK_NULL_HANDLE) {
      data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, data->_vertices);
      data->_indices_buffer = _renderer->getAPI()->createIndexBuffer(commandPool, data->_indices);
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
        memcpy(newData, data->_indices.data(), data->_indices_buffer.size);
        vkUnmapMemory(_renderer->getDevice(), *data->_indices_buffer.memory->getMemory());

        data->_indices_buffer.memory->unLock();
      }
    }

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); ++i) {
      mesh->getData()->_ubos[i].projection = _renderer->getPerspective();

      if (_texture_manager->getTextures().contains(mesh->getData()->_bump_map)) {
        auto const tex = _texture_manager->getTextures()[mesh->getData()->_bump_map];
        mesh->getData()->_ubos[i].tex_size = glm::vec2(tex.getWidth(), tex.getHeight());
      }
    }

    if (mesh->getStorageBuffers()->empty()) {

      Material material{};
      material.ambient = mesh->getMaterial().ambient;
      material.diffuse = mesh->getMaterial().diffuse;
      material.specular = mesh->getMaterial().specular;
      material.transmittance = mesh->getMaterial().transmittance;
      material.emission = mesh->getMaterial().emission;
      material.shi_ior_diss = glm::vec3(mesh->getMaterial().shininess,
        mesh->getMaterial().ior, mesh->getMaterial().illum);

      //PLP_DEBUG("ambient {}",  material.ambient.r);
      //PLP_DEBUG("diffuse {}",  material.diffuse.r);
      //PLP_DEBUG("specular {}",  material.specular.r);
      //PLP_DEBUG("shi_ior_diss {}",  material.shi_ior_diss.x);

      ObjectBuffer objectBuffer{};
      objectBuffer.point_lights[0] = _light_manager->getPointLights().at(0);
      objectBuffer.point_lights[1] = _light_manager->getPointLights().at(1);
      objectBuffer.spot_light = _light_manager->getSpotLights().at(0);
      objectBuffer.sun_light = _light_manager->getSunLight();
      objectBuffer.material = material;

      auto storageBuffer{ _renderer->getAPI()->createStorageBuffers(objectBuffer, commandPool) };

      mesh->setObjectBuffer(objectBuffer);
      mesh->addStorageBuffer(storageBuffer);
      mesh->setHasBufferStorage();

      //_renderer->updateStorageBuffer(mesh->getStorageBuffers()->at(0), objectBuffer);
    }

    unsigned int min{ 0 };
    unsigned int max{ 0 };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
      max = mesh->getData()->_ubos_offset.at(i);
      auto ubos = std::vector<UniformBufferObject>(mesh->getData()->_ubos.begin() + min, mesh->getData()->_ubos.begin() + max);

      min = max;
      if (ubos.empty()) continue;
      _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);
    }

    if (*mesh->getDescSet() == NULL) {

      createDescriptorSet(mesh);
      setPushConstants(mesh);
    }
    mesh->setIsDirty(false);

    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);
  }
}
