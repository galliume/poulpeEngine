module;

#include <fmt/format.h>

module Engine.Renderer.Vulkan.Text;

import std;

import Engine.Component.Mesh;
import Engine.Component.Texture;

import Engine.Core.Constants;
import Engine.Core.FreeType;
import Engine.Core.GLM;
import Engine.Core.MaterialTypes;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Vertex;
import Engine.Core.Volk;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  void Text::operator()(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;


    if (!mesh.isDirty()) return;

    std::vector<Vertex> vertices;

    auto const screen_width{
        static_cast<float>(renderer.getAPI()->getSwapChainExtent().width)};
    auto const screen_height{
      static_cast<float>(renderer.getAPI()->getSwapChainExtent().height) };

    float x { _position.x };
    float y { _position.y };

    auto const utf16_text{ fmt::detail::utf8_to_utf16(_text).str() };
    float const width{ static_cast<float>(render_context.atlas_width) };
    float const height{ static_cast<float>(render_context.atlas_height) };

    glm::vec4 const tangent { 1.0f, 0.0f, 0.0f, 1.0f };
    glm::vec4 const normal { 0.0f, 1.0f, 0.0f, 0.0f };

    for (auto c { utf16_text.begin() }; c != utf16_text.end(); c++) {

      FT_ULong const ft_char { static_cast<FT_ULong>(*c) };

      auto const glyph_index { FT_Get_Char_Index(render_context.face, ft_char) };
      auto const ch { render_context.characters[glyph_index]} ;

      constexpr float epsilon = 1e-6f;
      if (std::abs(ch.size.x) < epsilon && std::abs(ch.size.y) < epsilon) {
        x += 5.f;
        continue;
      }

      float xpos = x + ch.bearing.x * _scale;
      float ypos = isFlat()
        ? y - ch.bearing.y * _scale
        : y + ch.bearing.y * _scale;

      float w = ch.size.x * _scale;
      float h = ch.size.y * _scale;
      if (!isFlat()) h = -h;

      float u0{ ch.x_offset / width };
      float v0{ (ch.y_offset + ch.size.y) / height };
      float u1{ (ch.x_offset + ch.size.x) / width };
      float v1{ ch.y_offset / height };

      Vertex vertex_1{
          tangent,           // tangent
          _color,             // color (vec4)
          {xpos, ypos+h, 0.f, 1.f},// pos (vec3)
          normal,           // normal
          {u0, v0}           // texture_coord (vec2)
      };

      Vertex vertex_2{
          tangent, _color, {xpos, ypos, 0.f, 1.f},
          normal, {u0, v1}
      };

      Vertex vertex_3{
          tangent, _color, {xpos+w, ypos, 0.f, 1.f},
          normal, {u1, v1}
      };

      Vertex vertex_4{
          tangent, _color, {xpos, ypos+h, 0.f, 1.f},
          normal, {u0, v0}
      };

      Vertex vertex_5{
          tangent, _color, {xpos+w, ypos, 0.f, 1.f},
          normal, {u1, v1}
      };

      Vertex vertex_6{
          tangent, _color, {xpos+w, ypos+h, 0.f, 1.f},
          normal, {u1, v0},
      };

      vertices.emplace_back(vertex_1);
      vertices.emplace_back(vertex_2);
      vertices.emplace_back(vertex_3);
      vertices.emplace_back(vertex_4);
      vertices.emplace_back(vertex_5);
      vertices.emplace_back(vertex_6);

      x += static_cast<float>((ch.advance >> 6)) * _scale;
    }

    glm::mat4 projection = glm::ortho(
      0.0f, screen_width,
      screen_height, 0.0f
    );

    std::uint32_t options{ 0 };

    //@todo isFlat is not obvious as a name
    if (!isFlat()) {
      projection = renderer.getPerspective();
      options |= 1 << 0;
    }

    auto& mat { mesh.getMaterials().at(0) };
    mat.double_sided = true;
    mat.alpha_mode = 1.0;

    //@todo rename to
    mesh.setOptions(options);

    auto const& data = mesh.getData();

    data->_vertices = vertices;
    data->_texture_index = 0;

    auto cmd_pool = renderer.getAPI()->createCommandPool();

    if (data->_ubos.empty()) {
      data->_vertex_buffer = renderer.getAPI()->createVertexBuffer(vertices, renderer.getCurrentFrameIndex());

      std::vector<UniformBufferObject> ubos{};
      ubos.reserve(1);

      UniformBufferObject ubo;
      ubo.model = glm::mat4(1.0f);
      ubo.projection = projection;
      ubos.push_back(ubo);

      data->_ubos.resize(1);
      data->_ubos[0] = ubos;

      mesh.getData()->_ubos_offset.emplace_back(1);
      mesh.getUniformBuffers().emplace_back(renderer.getAPI()->createUniformBuffers(1, renderer.getCurrentFrameIndex()));

      mat.alpha_mode = 1.0;

      for (std::size_t i{ 0 }; i < mesh.getData()->_ubos.size(); i++) {
        std::ranges::for_each(mesh.getData()->_ubos.at(i), [&](auto& data_ubo) {
          data_ubo.projection = projection;
        });
      }
    } else {
      auto const image_index { renderer.getCurrentFrameIndex() };
      auto const current_offset { renderer.getAPI()->getCurrentStagingMemoryOffset(image_index) };
      //VkDeviceMemory staging_device_memory{ renderer.getAPI()->getStagingMemory(renderer.getCurrentFrameIndex()) };
      VkBuffer staging_buffer { renderer.getAPI()->getStagingBuffer(image_index) };
      VkDeviceSize const buffer_size { sizeof(Vertex) * vertices.size() };
      renderer.getAPI()->updateCurrentStagingMemoryOffset(buffer_size, image_index);

      void* void_data { renderer.getAPI()->getStagingMemoryPtr(image_index) };
      std::memcpy(static_cast<char*>(void_data) + current_offset, vertices.data(), static_cast<std::size_t>(buffer_size));

      renderer.getAPI()->copyBuffer(
        staging_buffer,
        data->_vertex_buffer.buffer,
        buffer_size,
        current_offset,
        0,
        renderer.getCurrentFrameIndex());

        data->_is_dirty = true;
        renderer.updateVertexBuffer(data, renderer.getCurrentFrameIndex());
    }

    vkDestroyCommandPool(renderer.getDevice(), cmd_pool, nullptr);

    if (!mesh.getData()->_ubos.empty()) {
      renderer.getAPI()->updateUniformBuffer(mesh.getUniformBuffers().at(0), &mesh.getData()->_ubos.at(0), renderer.getCurrentFrameIndex());
    }

    if (*mesh.getDescSet() == nullptr) {
      createDescriptorSet(renderer, mesh, render_context);
    }
    mesh.setIsDirty(false);
  }

  void Text::createDescriptorSet(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {

    Texture atlas { render_context.textures->at("_plp_font_atlas")};

    atlas.setSampler(renderer.getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      1u));

    if (atlas.getWidth() == 0) {
      atlas = render_context.textures->at(PLP_EMPTY);
    }

    auto const sampler = renderer.getAPI()->createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      1);

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(sampler, atlas.getImageView(), VK_IMAGE_LAYOUT_GENERAL);

    auto const pipeline = renderer.getPipeline(mesh.getShaderName());
    VkDescriptorSet descset = renderer.getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    std::array<VkWriteDescriptorSet, 2> desc_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;

    std::for_each(std::begin(mesh.getUniformBuffers()), std::end(mesh.getUniformBuffers()),
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
    desc_writes[1].descriptorCount = static_cast<std::uint32_t>(image_infos.size());
    desc_writes[1].pImageInfo = image_infos.data();

    vkUpdateDescriptorSets(
      renderer.getAPI()->getDevice(),
      static_cast<std::uint32_t>(desc_writes.size()),
      desc_writes.data(),
      0,
      nullptr);

    mesh.setDescSet(descset);
  }
}
