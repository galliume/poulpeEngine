module;
#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string_view>
#include <vector>
#include <volk.h>

module Poulpe.Renderer.Vulkan.ShadowMap;

import Poulpe.Component.Components;
import Poulpe.Component.Texture;
import Poulpe.Core.MeshTypes;
import Poulpe.Core.PlpTypedef;
import Poulpe.Renderer.RendererComponentTypes;

namespace Poulpe
{
  void ShadowMap::operator()(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    auto const& mesh = component_rendering_info.mesh;

    if (!mesh && !mesh->isDirty()) return;

    auto cmd_pool = renderer->getAPI()->createCommandPool();
    auto const ubo_count { 1 };

    for (size_t i{ 0 }; i < ubo_count; ++i) {
      Buffer uniformBuffer = renderer->getAPI()->createUniformBuffers(1, cmd_pool);
      mesh->getUniformBuffers().emplace_back(uniformBuffer);
    }
    
    auto const& data = mesh->getData();

    data->_vertex_buffer = renderer->getAPI()->createVertexBuffer(cmd_pool, data->_vertices);
    data->_indices_buffer = renderer->getAPI()->createIndexBuffer(cmd_pool, data->_indices);
    data->_texture_index = 0;

    vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& ubo) {
        ubo.projection = renderer->getPerspective();
      });
    }

    if (mesh->getData()->_ubos_offset.empty()) {
      std::ranges::for_each(mesh->getData()->_bones, [&](auto const& bone) {
        
        auto const& b{ bone.second };

        Buffer uniformBuffer = renderer->getAPI()->createUniformBuffers(b.weights.size(), cmd_pool);
        mesh->getUniformBuffers().emplace_back(uniformBuffer);
      });
    }

    createDescriptorSet(renderer, component_rendering_info);
    mesh->setIsDirty(false);
  }

  void ShadowMap::createDescriptorSet(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    auto const& mesh = component_rendering_info.mesh;
    auto const& tex{ component_rendering_info.textures.at(mesh->getData()->_textures.at(0)) };

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    std::vector<VkDescriptorImageInfo> depth_map_image_infos{};
    std::vector<VkDescriptorImageInfo> cube_maps_infos{};

    auto const pipeline = renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    for (size_t i{ 0 }; i < mesh->getUniformBuffers().size(); ++i) {

      renderer->getAPI()->updateDescriptorSets(
        mesh->getUniformBuffers(),
        *mesh->getStorageBuffers(),
        descset,
        image_infos,
        depth_map_image_infos,
        cube_maps_infos);
    }

    mesh->setDescSet(descset);
  }
}
