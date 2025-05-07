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
        *mesh->getObjectStorageBuffer(),
        *mesh->getBonesStorageBuffer(),
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

    auto cmd_pool = _renderer->getAPI()->createCommandPool();
    auto const ubo_count { 1 };

    for (size_t i{ 0 }; i < ubo_count; ++i) {
      Buffer uniformBuffer = _renderer->getAPI()->createUniformBuffers(1, cmd_pool);
      mesh->getUniformBuffers()->emplace_back(uniformBuffer);
    }
    
    auto const& data = mesh->getData();

    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(cmd_pool, data->_vertices);
    data->_indices_buffer = _renderer->getAPI()->createIndexBuffer(cmd_pool, data->_indices);
    data->_texture_index = 0;

    vkDestroyCommandPool(_renderer->getDevice(), cmd_pool, nullptr);

    for (auto i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& ubo) {
        ubo.projection = _renderer->getPerspective();
      });
    }

    if (mesh->getData()->_ubos_offset.empty()) {
      std::ranges::for_each(mesh->getData()->_bones, [&](auto const& bone) {
        
        auto const& b{ bone.second };

        Buffer uniformBuffer = _renderer->getAPI()->createUniformBuffers(b.weights.size(), cmd_pool);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);
      });
    }

    createDescriptorSet(mesh);
    setPushConstants(mesh);
    mesh->setIsDirty(false);
  }
}
