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

module Engine.Renderer.Vulkan.Water;

import Engine.Component.Components;
import Engine.Component.Texture;
import Engine.Component.Vertex;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  struct Texture_Region
  {
    int lowest_height;
    int optimal_height;
    int heighest_height;
  };

void Water::operator()(
  Renderer *const renderer,
  ComponentRenderingInfo const& component_rendering_info)
  {
    stage_flag_bits =
      VK_SHADER_STAGE_VERTEX_BIT 
      | VK_SHADER_STAGE_FRAGMENT_BIT
      | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
      | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

    auto const& mesh = component_rendering_info.mesh;

    if (!mesh && !mesh->isDirty()) return;

    Texture const& tex { component_rendering_info.textures.at(component_rendering_info.terrain_name)};

    std::vector<Vertex> vertices;
    int const width{ static_cast<int>(tex.getWidth()) };
    int const height{ static_cast<int>(tex.getHeight()) };

    int const rez{ 50 };
    int index{ 0 };


    for(auto i = 0; i < rez - 1; i++) {
      for(auto j = 0; j < rez - 1; j++) {

        index = i + j;
        float const y{ 0.0f };

        Vertex v{ 
          { -width/2.0f + width*i/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f}, glm::vec3(0.0f), {}, {}};

        Vertex v2{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f }, glm::vec3(0.0f), {}, {}};

        Vertex v3{ 
          {-width/2.0f + width*i/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f }, glm::vec3(0.0f), {}, {}};

        Vertex v4{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f }, glm::vec3(0.0f), {}, {}};

        vertices.push_back(v);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
      }
    }
    auto const& data = mesh->getData();
    auto cmd_pool = renderer->getAPI()->createCommandPool();

    std::vector<UniformBufferObject> ubos{};
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.projection = renderer->getPerspective();
    ubos.push_back(ubo);

    data->_ubos.push_back(ubos);
    data->_vertices = vertices;
    data->_vertex_buffer = renderer->getAPI()->createVertexBuffer(cmd_pool, vertices);
    data->_texture_index = 0;

    mesh->getMaterial().alpha_mode = 2.0;//BLEND
    mesh->getData()->_ubos_offset.emplace_back(1);
    mesh->getUniformBuffers().emplace_back(renderer->getAPI()->createUniformBuffers(1, cmd_pool));

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& data_ubo) {
        data_ubo.projection = renderer->getPerspective();
      });
    }

    vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);

    if (!mesh->getData()->_ubos.empty()) {
      renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers().at(0), &mesh->getData()->_ubos.at(0));
    }

    createDescriptorSet(renderer, component_rendering_info);
    mesh->setIsDirty(false);
  }
  
  void Water::createDescriptorSet(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    auto const& mesh = component_rendering_info.mesh;

    Texture tex { component_rendering_info.textures.at(PLP_EMPTY)};
    
    tex.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (tex.getWidth() == 0) {
      tex = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture texture_normal{ component_rendering_info.textures.at(PLP_WATER_NORMAL_1) };
      texture_normal.setSampler(renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_bump_wrap_mode_u,
      mesh->getMaterial().texture_bump_wrap_mode_v,
      1));

    if (texture_normal.getWidth() == 0) {
      texture_normal = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture texture_normal2{ component_rendering_info.textures.at(PLP_WATER_NORMAL_2) };
      texture_normal2.setSampler(renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_bump_wrap_mode_u,
      mesh->getMaterial().texture_bump_wrap_mode_v,
      1));

    if (texture_normal2.getWidth() == 0) {
      texture_normal2 = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture env { component_rendering_info.textures.at(component_rendering_info.skybox_name) };
    env.setSampler(renderer->getAPI()->createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    env.getMipLevels()));

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_infos.emplace_back(renderer->getDepthSamplers(), renderer->getDepthImageViews(), VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(texture_normal.getSampler(), texture_normal.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_infos.emplace_back(texture_normal2.getSampler(), texture_normal2.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
    image_infos.emplace_back(renderer->getCurrentSampler(), renderer->getCurrentImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);

    // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMALVK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_GENERAL
    // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
    // VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    // VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL
    // VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ

    std::vector<VkDescriptorImageInfo> env_image_infos{};
    env_image_infos.emplace_back(env.getSampler(), env.getImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL );

    auto const pipeline = renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    //renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, image_infos);

    std::array<VkWriteDescriptorSet, 3> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    std::for_each(std::begin(mesh->getUniformBuffers()), std::end(mesh->getUniformBuffers()),
    [& buffer_infos](const Buffer & uniformBuffer)
    {
      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = uniformBuffer.buffer;
      buffer_info.offset = 0;
      buffer_info.range = VK_WHOLE_SIZE;
      buffer_infos.emplace_back(buffer_info);
    });

    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = descset;
    desc_writes[0].dstBinding = 0;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].pBufferInfo = buffer_infos.data();

    desc_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[1].dstSet = descset;
    desc_writes[1].dstBinding = 1;
    desc_writes[1].dstArrayElement = 0;
    desc_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[1].descriptorCount = static_cast<uint32_t>(image_infos.size());
    desc_writes[1].pImageInfo = image_infos.data();

    desc_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[2].dstSet = descset;
    desc_writes[2].dstBinding = 2;
    desc_writes[2].dstArrayElement = 0;
    desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[2].descriptorCount = static_cast<uint32_t>(env_image_infos.size());
    desc_writes[2].pImageInfo = env_image_infos.data();

    vkUpdateDescriptorSets(
      renderer->getAPI()->getDevice(),
      static_cast<uint32_t>(desc_writes.size()),
      desc_writes.data(),
      0,
      nullptr);

    mesh->setDescSet(descset);
  }
}
