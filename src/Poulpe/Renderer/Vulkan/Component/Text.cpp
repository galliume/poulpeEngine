module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>


#include <algorithm>
#include <array>
#include <chrono>
#include <fmt/format.h>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string_view>
#include <vector>
#include <volk.h>

module Poulpe.Renderer.Vulkan.Text;

import Poulpe.Component.Components;
import Poulpe.Component.Texture;
import Poulpe.Component.Vertex;
import Poulpe.Core.MeshTypes;
import Poulpe.Core.PlpTypedef;
import Poulpe.Renderer.RendererComponentTypes;

namespace Poulpe
{
  void Text::operator()(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    auto const& mesh = component_rendering_info.mesh;

    if (!mesh && !mesh->isDirty()) return;
  
    std::vector<Vertex> vertices;

    float x { _position.x };
    float y { _position.y };

    auto const utf16_text{ fmt::detail::utf8_to_utf16(_text).str() };

    for (auto c = utf16_text.begin(); c != utf16_text.end(); c++) {

      auto const& glyph_index { FT_Get_Char_Index(component_rendering_info.face, *c) };
      auto const& ch { component_rendering_info.characters.at(glyph_index)} ;

      constexpr float epsilon = 1e-6f;
      if (std::abs(ch.size.x) < epsilon && std::abs(ch.size.y) < epsilon) {
        x += 5.f;
        continue;
      }

      float xpos = x + ch.bearing.x * _scale;
      float ypos = y - ch.bearing.y * _scale;

      float w = ch.size.x * _scale;
      float h = ch.size.y * _scale;

      float const width{ static_cast<float>(component_rendering_info.atlas_width) };
      float const height{ static_cast<float>(component_rendering_info.atlas_height) };

      float u0{ ch.x_offset / width };
      float v0{ (ch.y_offset + ch.size.y) / height };
      float u1{ (ch.x_offset + ch.size.x) / width };
      float v1{ ch.y_offset / height };
      
      Vertex vertex_1{
        { xpos, ypos + h, 0.0f},
        _color, { u0, v0 },
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {}, {}};

      Vertex vertex_2{
        { xpos, ypos, 0.0f},
        _color, { u0, v1 },
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {}, {}};

      Vertex vertex_3{
        { xpos + w, ypos, 0.0f},
        _color, { u1, v1 },
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {}, {}};

      Vertex vertex_4{
        { xpos, ypos + h, 0.0f},
        _color, { u0, v0 },
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {}, {}};

      Vertex vertex_5{
        { xpos + w, ypos, 0.0f},
        _color, { u1, v1 },
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {}, {}};

      Vertex vertex_6{
        { xpos + w, ypos + h, 0.0f},
        _color, { u1, v0 },
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        { 0.0f, 0.0f, 0.0f},
        {}, {}};

      vertices.emplace_back(vertex_1);
      vertices.emplace_back(vertex_2);
      vertices.emplace_back(vertex_3);
      vertices.emplace_back(vertex_4);
      vertices.emplace_back(vertex_5);
      vertices.emplace_back(vertex_6);

      x += static_cast<float>((ch.advance >> 6)) * _scale;
    }

    auto const width{
        static_cast<float>(renderer->getAPI()->getSwapChainExtent().width)};
    auto const height{
      static_cast<float>(renderer->getAPI()->getSwapChainExtent().height) };

    glm::mat4 projection{ glm::ortho(0.0f, width, 0.0f, height) };
    
    glm::vec4 options{ 0.0f };
    
    if (!isFlat()) {
      projection = renderer->getPerspective();

      options.x = 1.0f;
      mesh->getMaterial().double_sided = true;
    }
    
    mesh->setOptions(options);

    auto const& data = mesh->getData();

    data->_vertices = vertices;
    data->_texture_index = 0;

    auto cmd_pool = renderer->getAPI()->createCommandPool();

    if (data->_ubos.empty()) {
      data->_vertex_buffer = renderer->getAPI()->createVertexBuffer(cmd_pool, vertices);

      std::vector<UniformBufferObject> ubos{};
      ubos.reserve(1);

      UniformBufferObject ubo;
      ubo.model = glm::mat4(1.0f);
      ubo.projection = projection;
      ubos.push_back(ubo);

      data->_ubos.resize(1);
      data->_ubos[0] = ubos;

      mesh->getData()->_ubos_offset.emplace_back(1);
      mesh->getUniformBuffers().emplace_back(renderer->getAPI()->createUniformBuffers(1, cmd_pool));
      mesh->getMaterial().alpha_mode = 1.0;

      for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
        std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& data_ubo) {
          data_ubo.projection = projection;
        });
      }
    } 
    else {
      VkDeviceMemory staging_device_memory{};
      VkDeviceSize buffer_size = sizeof(Vertex) * vertices.size();

      VkBuffer staging_buffer{};

      renderer->getAPI()->createBuffer(
       buffer_size,
       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
         | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
         | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
       staging_buffer, staging_device_memory);

      void* void_data;
      vkMapMemory(renderer->getDevice(), staging_device_memory, 0, buffer_size, 0, &void_data);
      memcpy(void_data, vertices.data(), static_cast<size_t>(buffer_size));
      vkUnmapMemory(renderer->getDevice(), staging_device_memory);

      renderer->getAPI()->copyBuffer(
        cmd_pool,
        staging_buffer,
        data->_vertex_buffer.buffer,
        buffer_size,
        0);

      vkDestroyBuffer(renderer->getDevice(), staging_buffer, nullptr);
      vkFreeMemory(renderer->getDevice(), staging_device_memory, nullptr);
    }

    vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);

    if (!mesh->getData()->_ubos.empty()) {
      renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers().at(0), &mesh->getData()->_ubos.at(0));
    }
    
    if (*mesh->getDescSet() == nullptr) {
      createDescriptorSet(renderer, component_rendering_info);
    } 
    mesh->setIsDirty(false);
  }

  void Text::createDescriptorSet(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    auto const& mesh = component_rendering_info.mesh;

    Texture atlas { component_rendering_info.textures.at("_plp_font_atlas")};
    
    atlas.setSampler(renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (atlas.getWidth() == 0) {
      atlas = component_rendering_info.textures.at(PLP_EMPTY);
    }

    auto const sampler = renderer->getAPI()->createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      1);
 
    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(sampler, atlas.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    auto const pipeline = renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    std::array<VkWriteDescriptorSet, 2> desc_writes{};
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

    vkUpdateDescriptorSets(
      renderer->getAPI()->getDevice(),
      static_cast<uint32_t>(desc_writes.size()),
      desc_writes.data(),
      0,
      nullptr);

    mesh->setDescSet(descset);
  }
}
