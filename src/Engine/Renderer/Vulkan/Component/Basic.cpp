module;

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <functional>
#include <string_view>
#include <vector>
#include <volk.h>

module Engine.Renderer.Vulkan.Basic;

import Engine.Component.Components;
import Engine.Component.Texture;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  void Basic::operator()(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    auto const& mesh = component_rendering_info.mesh;

    if (!mesh && !mesh->isDirty()) return;

    auto cmd_pool = renderer->getAPI()->createCommandPool();

    if (mesh->getUniformBuffers().empty()) {
      std::ranges::for_each(mesh->getData()->_bones, [&](auto const&) {
        
        //auto const& b{ bone.second };

        Buffer uniformBuffer = renderer->getAPI()->createUniformBuffers(1);
        mesh->getUniformBuffers().emplace_back(std::move(uniformBuffer));
      });
    }
    if (mesh->getUniformBuffers().empty()) { //no bones
      Buffer uniformBuffer = renderer->getAPI()->createUniformBuffers(1);
      mesh->getUniformBuffers().emplace_back(std::move(uniformBuffer));
    }

    auto const& data = mesh->getData();
    data->_texture_index = 0;

    if (data->_vertex_buffer.buffer == VK_NULL_HANDLE) {
      data->_vertex_buffer = renderer->getAPI()->createVertexBuffer(data->_vertices);
      data->_indices_buffer = renderer->getAPI()->createIndexBuffer(data->_indices);
    } else {
      //suppose we have to update data
      {
        data->_vertex_buffer.memory->lock();
        auto *buffer { data->_vertex_buffer.memory->getBuffer(data->_vertex_buffer.index) };
        renderer->getAPI()->updateVertexBuffer(data->_vertices, buffer);
        data->_vertex_buffer.memory->unLock();
      }
    }

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& ubo) {
        ubo.projection = renderer->getPerspective();
      });
    }

    if (mesh->getStorageBuffers()->empty()) {

      Material material{};
      material.base_color = mesh->getMaterial().base_color;
      material.ambient = mesh->getMaterial().ambient;
      material.diffuse = mesh->getMaterial().diffuse;
      material.specular = mesh->getMaterial().specular;
      material.transmittance = mesh->getMaterial().transmittance;
      material.mre_factor = mesh->getMaterial().mre_factor;
      material.emissive_color = mesh->getMaterial().emissive_color;

      material.shi_ior_diss = glm::vec3(
        mesh->getMaterial().shininess,
        mesh->getMaterial().ior,
        mesh->getMaterial().dissolve);

      material.alpha = glm::vec3(mesh->getMaterial().alpha_mode, mesh->getMaterial().alpha_cut_off, 1.0);

      //@todo needed to modify assimp sources glTF2Asset.h textureTransformSupported = true (preview ?)
      material.ambient_translation = mesh->getMaterial().ambient_translation;
      material.ambient_scale = mesh->getMaterial().ambient_scale;
      material.ambient_rotation = { mesh->getMaterial().ambient_rotation.x, mesh->getMaterial().ambient_rotation.y, 1.0 };

      material.normal_translation = mesh->getMaterial().normal_translation;
      material.normal_scale = mesh->getMaterial().normal_scale;
      material.normal_rotation = { mesh->getMaterial().normal_rotation.x, mesh->getMaterial().normal_rotation.y, 1.0 };

      material.diffuse_translation = mesh->getMaterial().diffuse_translation;
      material.diffuse_scale = mesh->getMaterial().diffuse_scale;
      material.diffuse_rotation = { mesh->getMaterial().diffuse_rotation.x, mesh->getMaterial().diffuse_rotation.y, 1.0 };

      material.emissive_translation = mesh->getMaterial().emissive_translation;
      material.emissive_scale = mesh->getMaterial().emissive_scale;
      material.emissive_rotation = { mesh->getMaterial().emissive_rotation.x, mesh->getMaterial().emissive_rotation.y, 1.0 };

      material.mr_translation = mesh->getMaterial().mr_translation;
      material.mr_scale = mesh->getMaterial().mr_scale;
      material.mr_rotation = { mesh->getMaterial().mr_rotation.x, mesh->getMaterial().mr_rotation.y, 1.0 };

      material.strength = { mesh->getMaterial().normal_strength, mesh->getMaterial().occlusion_strength, 0.0 };//x normal strength, y occlusion strength

      //Logger::debug("ambient {}",  material.ambient.r);
      //Logger::debug("diffuse {}",  material.diffuse.r);
      //Logger::debug("specular {}",  material.specular.r);
      //Logger::debug("shi_ior_diss {} {}", material.shi_ior_diss.x, material.shi_ior_diss.y);

      ObjectBuffer objectBuffer{};
      objectBuffer.point_lights[0] = component_rendering_info.point_lights.at(0);
      objectBuffer.point_lights[1] = component_rendering_info.point_lights.at(1);
      objectBuffer.spot_light = component_rendering_info.spot_lights.at(0);
      objectBuffer.sun_light = component_rendering_info.sun_light;
      objectBuffer.material = material;

      auto storageBuffer{ renderer->getAPI()->createStorageBuffers(objectBuffer) };

      mesh->setObjectBuffer(objectBuffer);
      mesh->addStorageBuffer(storageBuffer);
      mesh->setHasBufferStorage();

      //renderer->updateStorageBuffer(mesh->getStorageBuffers()->at(0), objectBuffer);
    }

    for (size_t i{ 0 }; i < mesh->getUniformBuffers().size(); ++i) {

      auto& ubos{ mesh->getUniformBuffers().at(i) };
      auto& ubos_data{ mesh->getData()->_ubos.at(i) };

      renderer->getAPI()->updateUniformBuffer(ubos, &ubos_data);
    }

    if (*mesh->getDescSet() == nullptr) {
      createDescriptorSet(renderer, component_rendering_info);
    }
    mesh->setIsDirty(false);

    vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);
  }
  
  void Basic::createDescriptorSet(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    auto const& mesh = component_rendering_info.mesh;

    Texture tex { component_rendering_info.textures.at(mesh->getData()->_textures.at(0)) };
    tex.setSampler(renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_diffuse_wrap_mode_u,
      mesh->getMaterial().texture_diffuse_wrap_mode_v,
      tex.getMipLevels()));

    Texture alpha { component_rendering_info.textures.at(mesh->getData()->_alpha) };
    alpha.setSampler(renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_alpha_wrap_mode_u,
      mesh->getMaterial().texture_alpha_wrap_mode_v,
      alpha.getMipLevels()));

    std::string const bump_map_name{ mesh->getData()->_bump_map };
    Texture texture_bump{ component_rendering_info.textures.at(bump_map_name) };
      texture_bump.setSampler(renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_bump_wrap_mode_u,
      mesh->getMaterial().texture_bump_wrap_mode_v,
      1));

    if (texture_bump.getWidth() == 0) {
      texture_bump = component_rendering_info.textures.at(PLP_EMPTY);
    }

    std::string specular_map_name{ mesh->getData()->_specular_map };
    Texture texture_specular{ component_rendering_info.textures.at(specular_map_name)};
    texture_specular.setSampler(renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_specular_wrap_mode_u,
    mesh->getMaterial().texture_specular_wrap_mode_v,
    texture_specular.getMipLevels()));

    if (texture_specular.getWidth() == 0) {
      texture_specular = component_rendering_info.textures.at(PLP_EMPTY);
    }

    std::string metal_roughness_map_name{ mesh->getData()->_metal_roughness};
    Texture texture_metal_roughness { component_rendering_info.textures.at(metal_roughness_map_name) };
    texture_metal_roughness.setSampler(renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_metal_roughness_wrap_mode_u,
    mesh->getMaterial().texture_metal_roughness_wrap_mode_v,
    texture_metal_roughness.getMipLevels()));

    if (texture_metal_roughness.getWidth() == 0) {
      texture_metal_roughness = component_rendering_info.textures.at(PLP_EMPTY);
    }

    std::string emissive_map_name{ mesh->getData()->_emissive};
    Texture texture_emissive { component_rendering_info.textures.at(emissive_map_name) };
    texture_emissive.setSampler(renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_emissive_wrap_mode_u,
    mesh->getMaterial().texture_emissive_wrap_mode_v,
    texture_emissive.getMipLevels()));

    if (texture_emissive.getWidth() == 0) {
      texture_emissive = component_rendering_info.textures.at(PLP_EMPTY);
    }

    std::string ao_map_name{ mesh->getData()->_ao};
    Texture texture_ao { component_rendering_info.textures.at(ao_map_name) };
    texture_ao.setSampler(renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_ao_wrap_mode_u,
    mesh->getMaterial().texture_ao_wrap_mode_v,
    texture_ao.getMipLevels()));

    if (texture_ao.getWidth() == 0) {
      texture_ao = component_rendering_info.textures.at(PLP_EMPTY);
    }

    std::string base_color_map_name{ mesh->getData()->_base_color};
    Texture texture_base_color { component_rendering_info.textures.at(base_color_map_name) };
    texture_base_color.setSampler(renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_base_color_wrap_mode_u,
    mesh->getMaterial().texture_base_color_wrap_mode_v,
    texture_base_color.getMipLevels()));

    if (texture_base_color.getWidth() == 0) {
      texture_base_color = component_rendering_info.textures.at(PLP_EMPTY);
    }

    //VkDescriptorImageInfo shadowMapSpot{};
    //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //shadowMapSpot.imageview = renderer->getDepthMapImageViews()->at(1);
    //shadowMapSpot.sampler = renderer->getDepthMapSamplers()->at(1);
    std::vector<VkDescriptorImageInfo> image_info{};
    image_info.reserve(7);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(alpha.getSampler(), alpha.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(texture_bump.getSampler(), texture_bump.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(texture_specular.getSampler(), texture_specular.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(texture_metal_roughness.getSampler(), texture_metal_roughness.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(texture_emissive.getSampler(), texture_emissive.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(texture_ao.getSampler(), texture_ao.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);

    std::vector<VkDescriptorImageInfo> env_info{};

    Texture texture_environment{ component_rendering_info.textures.at(component_rendering_info.skybox_name) };
    texture_environment.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      0));

    if (texture_environment.getWidth() == 0) {
      texture_environment = component_rendering_info.textures.at(PLP_EMPTY);
    }
    env_info.emplace_back(texture_environment.getSampler(), texture_environment.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);

    std::vector<VkDescriptorImageInfo> depth_map_image_info{};
    depth_map_image_info.emplace_back(renderer->getDepthMapSamplers(), renderer->getDepthMapImageViews(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    //image_info.emplace_back(shadowMapSpot);

    auto const& pipeline = renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset{ renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1) };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers().size(); ++i) {

      renderer->getAPI()->updateDescriptorSets(
        mesh->getUniformBuffers(),
        *mesh->getStorageBuffers(),
        descset, image_info, depth_map_image_info, env_info);
    }

    mesh->setDescSet(descset);

    // std::array<VkWriteDescriptorSet, 2> descset_writes{};
    // std::vector<VkDescriptorBufferInfo> buffer_infos;
    // std::vector<VkDescriptorBufferInfo> buffer_storage_infos;

    // std::for_each(std::begin(mesh->getUniformBuffers()), std::end(mesh->getUniformBuffers()),
    //   [&buffer_infos](const Buffer& uniformBuffer)
    //   {
    //     VkDescriptorBufferInfo buffer_info{};
    //     buffer_info.buffer = uniformBuffer.buffer;
    //     buffer_info.offset = 0;
    //     buffer_info.range = VK_WHOLE_SIZE;
    //     buffer_infos.emplace_back(buffer_info);
    //   });

    // auto const& shadow_map_pipeline = renderer->getPipeline("shadowMap");
    // VkDescriptorSet shadow_map_descset = renderer->getAPI()->createDescriptorSets(shadow_map_pipeline->desc_pool, { shadow_map_pipeline->descset_layout }, 1);

    // std::for_each(std::begin(*mesh->getStorageBuffers()), std::end(*mesh->getStorageBuffers()),
    //   [&buffer_storage_infos](const Buffer& storageBuffers)
    //   {
    //     VkDescriptorBufferInfo buffer_info{};
    //     buffer_info.buffer = storageBuffers.buffer;
    //     buffer_info.offset = 0;
    //     buffer_info.range = VK_WHOLE_SIZE;
    //     buffer_storage_infos.emplace_back(buffer_info);
    //   });

    // descset_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descset_writes[0].dstSet = shadow_map_descset;
    // descset_writes[0].dstBinding = 0;
    // descset_writes[0].dstArrayElement = 0;
    // descset_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // descset_writes[0].descriptorCount = 1;
    // descset_writes[0].pBufferInfo = buffer_infos.data();

    // descset_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // descset_writes[1].dstSet = shadow_map_descset;
    // descset_writes[1].dstBinding = 1;
    // descset_writes[1].dstArrayElement = 0;
    // descset_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    // descset_writes[1].descriptorCount = static_cast<uint32_t>(buffer_storage_infos.size());
    // descset_writes[1].pBufferInfo = buffer_storage_infos.data();

    // vkUpdateDescriptorSets(renderer->getDevice(), static_cast<uint32_t>(descset_writes.size()), descset_writes.data(), 0, nullptr);

    // mesh->setShadowMapDescSet(shadow_map_descset);
  }
}
