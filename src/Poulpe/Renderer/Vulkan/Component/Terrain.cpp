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

module Poulpe.Renderer;

import Poulpe.Component.Components;
import Poulpe.Component.Texture;
import Poulpe.Component.Vertex;
import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  struct Texture_Region
  {
    int lowest_height;
    int optimal_height;
    int heighest_height;
  };

  void Terrain::operator()(
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

    Texture const& tex { component_rendering_info.textures.at(component_rendering_info.terrain_name) };

    std::vector<Vertex> vertices;
    uint32_t const width{ tex.getWidth() };
    uint32_t const height{ tex.getHeight()};

    uint32_t const rez{ 20 };

    for(size_t i = 0; i < rez - 1; i++) {
      for(size_t j = 0; j < rez - 1; j++) {

        float const y{ 0.0f };

        Vertex v{ 
          { -width/2.0f + width*i/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f},
          glm::vec3{0.0f}, {}, {}};

        Vertex v2{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f, 0.0f },
          glm::vec3{0.0f}, {}, {} };

        Vertex v3{ 
          {-width/2.0f + width*i/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f, 0.0f },
          glm::vec3{0.0f}, {}, {}};

        Vertex v4{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f, 0.0f },
          glm::vec3{0.0f}, {}, {} };

        vertices.push_back(v);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
      }
    }
    auto const& data = mesh->getData();
    auto commandPool = renderer->getAPI()->createCommandPool();

    std::vector<UniformBufferObject> ubos{};
    ubos.reserve(1);
    UniformBufferObject ubo;
    ubo.model = glm::mat4(1.0f);
    ubo.projection = renderer->getPerspective();
    ubos.push_back(ubo);

    data->_vertices = vertices;
    data->_vertex_buffer = renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data->_texture_index = 0;
    data->_ubos.resize(1);
    data->_ubos[0] = ubos;

    mesh->getData()->_ubos_offset.emplace_back(1);
    mesh->getUniformBuffers()->emplace_back(renderer->getAPI()->createUniformBuffers(1, commandPool));

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& data_ubo) {
        data_ubo.projection = renderer->getPerspective();
      });
    }

    vkDestroyCommandPool(renderer->getDevice(), commandPool, nullptr);

    if (!mesh->getData()->_ubos.empty()) {
      renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(0), &mesh->getData()->_ubos.at(0));
    }

    createDescriptorSet(renderer, component_rendering_info);
    mesh->setIsDirty(false);
  }

  void Terrain::createDescriptorSet(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    auto const& mesh = component_rendering_info.mesh;
    auto height_map { component_rendering_info.textures.at(component_rendering_info.terrain_name)};
    
    height_map.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (height_map.getWidth() == 0) {
      height_map = component_rendering_info.textures.at(PLP_EMPTY);
    }

    //@todo fix this ugly fix. Needs a real asset unique ID
    Texture ground { component_rendering_info.textures.at(PLP_GROUND)};
    
    ground.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (ground.getWidth() == 0) {
      ground = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture grass { component_rendering_info.textures.at(PLP_GRASS)};
    
    grass.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (grass.getWidth() == 0) {
      grass = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture snow { component_rendering_info.textures.at(PLP_SNOW)};
    
    snow.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (snow.getWidth() == 0) {
      snow = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture sand { component_rendering_info.textures.at(PLP_SAND)};

    sand.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (sand.getWidth() == 0) {
      sand = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture low_noise { component_rendering_info.textures.at(PLP_LOW_NOISE)};

    low_noise.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (low_noise.getWidth() == 0) {
      low_noise = component_rendering_info.textures.at(PLP_EMPTY);
    }

    Texture hi_noise { component_rendering_info.textures.at(PLP_HI_NOISE)};

    hi_noise.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (hi_noise.getWidth() == 0) {
      hi_noise = component_rendering_info.textures.at(PLP_EMPTY);
    }

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(height_map.getSampler(), height_map.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(ground.getSampler(), ground.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(grass.getSampler(), grass.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(snow.getSampler(), snow.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(sand.getSampler(), sand.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(hi_noise.getSampler(), hi_noise.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(low_noise.getSampler(), low_noise.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    Texture env { component_rendering_info.textures.at(component_rendering_info.skybox_name) };
    env.setSampler(renderer->getAPI()->createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    env.getMipLevels()));

    std::vector<VkDescriptorImageInfo> env_image_infos{};
    env_image_infos.emplace_back(env.getSampler(), env.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    auto const pipeline = renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    //renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, image_infos);
    std::array<VkWriteDescriptorSet, 3> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    std::for_each(std::begin(*mesh->getUniformBuffers()), std::end(*mesh->getUniformBuffers()),
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
