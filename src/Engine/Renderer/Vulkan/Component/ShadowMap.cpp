module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <volk.h>

module Engine.Renderer.Vulkan.ShadowMap;

import std;

import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  void ShadowMap::operator()(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT;

    // auto const& mesh = component_rendering_info.mesh;

    // if (!mesh && !mesh->isDirty()) return;

    // auto cmd_pool = renderer->getAPI()->createCommandPool();
    // auto const ubo_count { 1 };

    // for (std::size_t i{ 0 }; i < ubo_count; ++i) {
    //   Buffer uniformBuffer = renderer->getAPI()->createUniformBuffers(1, cmd_pool);
    //   mesh->getUniformBuffers().emplace_back(uniformBuffer);
    // }

    // auto const& data = mesh->getData();

    // data->_vertex_buffer = renderer->getAPI()->createVertexBuffer(cmd_pool, data->_vertices);
    // data->_indices_buffer = renderer->getAPI()->createIndexBuffer(cmd_pool, data->_indices);
    // data->_texture_index = 0;

    // vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);

    // for (std::size_t i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
    //   std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& ubo) {
    //     ubo.projection = renderer->getPerspective();
    //   });
    // }

      // Material material{};

      // ObjectBuffer objectBuffer{};
      // objectBuffer.point_lights[0] = component_rendering_info.point_lights.at(0);
      // objectBuffer.point_lights[1] = component_rendering_info.point_lights.at(1);
      // objectBuffer.spot_light = component_rendering_info.spot_lights.at(0);
      // objectBuffer.sun_light = component_rendering_info.sun_light;
      // objectBuffer.material = material;

      // auto storageBuffer{ renderer->getAPI()->createStorageBuffers(objectBuffer, cmd_pool) };

      // mesh->setObjectBuffer(objectBuffer);
      // mesh->addStorageBuffer(storageBuffer);
      // mesh->setHasBufferStorage();

    // if (mesh->getData()->_ubos_offset.empty()) {
    //   std::ranges::for_each(mesh->getData()->_bones, [&](auto const& bone) {

    //     auto const& b{ bone.second };

    //     Buffer uniformBuffer = renderer->getAPI()->createUniformBuffers(b.weights.size(), cmd_pool);
    //     mesh->getUniformBuffers().emplace_back(uniformBuffer);
    //   });
    // }

    createDescriptorSet(renderer, component_rendering_info);
    //mesh->setIsDirty(false);
  }

  void ShadowMap::createDescriptorSet(
    Renderer *const,
    ComponentRenderingInfo const&)
  {
    // auto const& mesh = component_rendering_info.mesh;
    // auto const& tex{ component_rendering_info.textures->at(mesh->getData()->_textures.at(0)) };

    // std::vector<VkDescriptorImageInfo> image_infos{};
    // image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    // std::vector<VkDescriptorImageInfo> depth_map_image_infos{};
    // std::vector<VkDescriptorImageInfo> cube_maps_infos{};

    // auto const pipeline = renderer->getPipeline(mesh->getShaderName());
    // VkDescriptorSet descset = renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    // for (std::size_t i{ 0 }; i < mesh->getUniformBuffers().size(); ++i) {

    //   renderer->getAPI()->updateDescriptorSets(
    //     mesh->getUniformBuffers(),
    //     *mesh->getStorageBuffers(),
    //     descset,
    //     image_infos,
    //     depth_map_image_infos,
    //     cube_maps_infos);
    // }

    // mesh->setDescSet(descset);
  }
}
