#include "ShadowMap.hpp"

namespace Poulpe
{
  struct constants;

  void ShadowMap::createDescriptorSet(Mesh* mesh)
  {
    Texture const tex{ _texture_manager->getTextures()[mesh->getData()->_textures.at(0)] };

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    std::vector<VkDescriptorImageInfo> depth_map_image_infos{};
    std::vector<VkDescriptorImageInfo> cube_maps_infos{};

    auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {

      _renderer->getAPI()->updateDescriptorSets(
        *mesh->getUniformBuffers(),
        *mesh->getStorageBuffers(),
        descset,
        image_infos,
        depth_map_image_infos,
        cube_maps_infos);
    }

    mesh->setDescSet(descset);
  }

  void ShadowMap::setPushConstants(Mesh* mesh)
  {
    mesh->setApplyPushConstants([](
      VkCommandBuffer & cmd_buffer,
      VkPipelineLayout pipeline_layout,
      Renderer* const renderer, Mesh* const meshS) {

      constants push_constants{};
      push_constants.view = renderer->getCamera()->lookAt();
      push_constants.view_position = renderer->getCamera()->getPos();

      vkCmdPushConstants(
        cmd_buffer,
        pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(constants),
        &push_constants);
    });

    mesh->setHasPushConstants();
  }

  void ShadowMap::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;

    uint32_t const totalInstances{ static_cast<uint32_t>(mesh->getData()->_ubos.size()) };
    uint32_t const maxUniformBufferRange{ _renderer->getAPI()->getDeviceProperties().limits.maxUniformBufferRange };
    unsigned long long const uniformBufferChunkSize{ maxUniformBufferRange / sizeof(UniformBufferObject) };
    uint32_t const uniformBuffersCount{ static_cast<uint32_t>(std::ceil(static_cast<float>(totalInstances) / static_cast<float>(uniformBufferChunkSize))) };

    //@todo fix memory management...
    unsigned long long uboOffset{ (totalInstances > uniformBufferChunkSize) ? uniformBufferChunkSize : totalInstances };
    unsigned long long uboRemaining { (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0};
    unsigned long long nbUbo { uboOffset};

    auto commandPool = _renderer->getAPI()->createCommandPool();

    for (size_t i{ 0 }; i < uniformBuffersCount; ++i) {

      mesh->getData()->_ubos_offset.emplace_back(uboOffset);
      Buffer uniformBuffer = _renderer->getAPI()->createUniformBuffers(nbUbo, commandPool);
      mesh->getUniformBuffers()->emplace_back(uniformBuffer);

      uboOffset = (uboRemaining > uniformBufferChunkSize) ? uboOffset + uniformBufferChunkSize : uboOffset + uboRemaining;
      nbUbo = (uboRemaining > uniformBufferChunkSize) ? uniformBufferChunkSize : uboRemaining;
      uboRemaining = (totalInstances - uboOffset > 0) ? totalInstances - uboOffset : 0;
    }

    auto const& data = mesh->getData();

    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, data->_vertices);
    data->_indices_buffer = _renderer->getAPI()->createIndexBuffer(commandPool, data->_indices);
    data->_texture_index = 0;

    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); ++i) {
      mesh->getData()->_ubos[i].projection = _renderer->getPerspective();
    }

    unsigned int min{ 0 };
    unsigned int max{ 0 };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
      max = mesh->getData()->_ubos_offset.at(i);
      auto ubos = std::vector<UniformBufferObject>(mesh->getData()->_ubos.begin() + min, mesh->getData()->_ubos.begin() + max);

      _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);

      min = max;
    }

    createDescriptorSet(mesh);
    setPushConstants(mesh);
    mesh->setIsDirty(false);
  }
}
