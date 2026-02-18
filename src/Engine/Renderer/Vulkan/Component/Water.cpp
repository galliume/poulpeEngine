module Engine.Renderer.Vulkan.Water;

import std;

import Engine.Component.Mesh;
import Engine.Component.Texture;

import Engine.Core.Constants;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Vertex;
import Engine.Core.Volk;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  struct Texture_Region
  {
    int lowest_height;
    int optimal_height;
    int heighest_height;
  };

void Water::operator()(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {
    stage_flag_bits =
      VK_SHADER_STAGE_VERTEX_BIT
      | VK_SHADER_STAGE_FRAGMENT_BIT
      | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
      | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;


    if (!mesh.isDirty()) return;

    std::vector<Vertex> vertices;
    int const width{ static_cast<int>(render_context.terrain->getWidth()) };
    int const height{ static_cast<int>(render_context.terrain->getHeight()) };

    int const rez{ 50 };
    int index{ 0 };
    float const fRez { static_cast<float>(rez) };
    glm::vec4 const tangentDefault{ 1.0f, 0.0f, 0.0f, 1.0f };
    glm::vec4 const normalDefault{ 0.0f, 1.0f, 0.0f, 0.0f };


    for(auto i = 0; i < rez - 1; i++) {
      for(auto j = 0; j < rez - 1; j++) {

        index = i + j;
        float const y{ 0.0f };

        // Vertex v1
        Vertex v{
            tangentDefault,                      // tangent (vec4 - 16 bytes)
            { (float)index, 0.0f, 0.0f, 0.0f },  // color (vec4 - 16 bytes)
            { -width/2.0f + width*i/fRez, y, -height/2.0f + height*j/fRez, 1.0f }, // pos (vec3)
            normalDefault,                       // normal (vec3)
            { i / fRez, j / fRez }              // texture_coord (vec2)
        };

        // Vertex v2
        Vertex v2{
            tangentDefault,
            { (float)index, 0.0f, 0.0f, 0.0f },
            { -width/2.0f + width*(i+1)/fRez, y, -height/2.0f + height*j/fRez, 1.0f },
            normalDefault,
            { (i+1) / fRez, j / fRez }
        };

        // Vertex v3
        Vertex v3{
            tangentDefault,
            { (float)index, 0.0f, 0.0f, 0.0f },
            { -width/2.0f + width*i/fRez, y, -height/2.0f + height*(j+1)/fRez, 1.0f },
            normalDefault,
            { i / fRez, (j+1) / fRez }
        };

        // Vertex v4
        Vertex v4{
            tangentDefault,
            { (float)index, 0.0f, 0.0f, 0.0f },
            { -width/2.0f + width*(i+1)/fRez, y, -height/2.0f + height*(j+1)/fRez, 1.0f },
            normalDefault,
            { (i+1) / fRez, (j+1) / fRez }
        };

        vertices.push_back(v);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
      }
    }
    auto const& data = mesh.getData();
    auto cmd_pool = renderer.getAPI()->createCommandPool();

    std::vector<UniformBufferObject> ubos{};
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.projection = renderer.getPerspective();
    ubos.push_back(ubo);

    data->_ubos.push_back(ubos);
    data->_vertices = vertices;
    data->_vertex_buffer = renderer.getAPI()->createVertexBuffer(vertices, renderer.getCurrentFrameIndex());
    data->_texture_index = 0;

    auto& mat { mesh.getMaterials().at(0) };
    mat.alpha_mode = 2.0;//BLEND

    mesh.getData()->_ubos_offset.emplace_back(1);
    mesh.getUniformBuffers().emplace_back(renderer.getAPI()->createUniformBuffers(1, renderer.getCurrentFrameIndex()));

    for (std::size_t i{ 0 }; i < mesh.getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh.getData()->_ubos.at(i), [&](auto& data_ubo) {
        data_ubo.projection = renderer.getPerspective();
      });
    }

    vkDestroyCommandPool(renderer.getDevice(), cmd_pool, nullptr);

    if (!mesh.getData()->_ubos.empty()) {
      renderer.getAPI()->updateUniformBuffer(mesh.getUniformBuffers().at(0), &mesh.getData()->_ubos.at(0), renderer.getCurrentFrameIndex());
    }

    createDescriptorSet(renderer, mesh, render_context);
    mesh.setIsDirty(false);
  }

  void Water::createDescriptorSet(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {

    Texture tex { render_context.textures->at(PLP_EMPTY)};

    tex.setSampler(renderer.getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (tex.getWidth() == 0) {
      tex = render_context.textures->at(PLP_EMPTY);
    }

    Texture texture_normal{ render_context.textures->at(PLP_WATER_NORMAL_1) };
      texture_normal.setSampler(renderer.getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      1));

    if (texture_normal.getWidth() == 0) {
      texture_normal = render_context.textures->at(PLP_EMPTY);
    }

    Texture texture_normal2{ render_context.textures->at(PLP_WATER_NORMAL_2) };
      texture_normal2.setSampler(renderer.getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      1));

    if (texture_normal2.getWidth() == 0) {
      texture_normal2 = render_context.textures->at(PLP_EMPTY);
    }

    render_context.skybox->setSampler(renderer.getAPI()->createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    render_context.skybox->getMipLevels()));

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_infos.emplace_back(renderer.getDepthSamplers(), renderer.getDepthImageViews(), VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(texture_normal.getSampler(), texture_normal.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_infos.emplace_back(texture_normal2.getSampler(), texture_normal2.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_infos.emplace_back(renderer.getCurrentSampler(), renderer.getCurrentImageView(), VK_IMAGE_LAYOUT_GENERAL);

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
    env_image_infos.emplace_back(render_context.skybox->getSampler(), render_context.skybox->getImageView(), VK_IMAGE_LAYOUT_GENERAL );

    std::vector<VkDescriptorImageInfo> csm_image_info{};
    csm_image_info.emplace_back(renderer.getCSMSamplers(), renderer.getCSMImageViews(), VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorImageInfo> depth_map_image_info{};
    depth_map_image_info.emplace_back(renderer.getDepthMapSamplers(), renderer.getDepthMapImageViews(), VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorSet descset {
       renderer.getAPI()->createDescriptorSets(renderer.getPipeline(mesh.getShaderName()), 1) };

    auto light_buffer {render_context.light_buffer};

    //renderer.getAPI()->updateDescriptorSets(*mesh.getUniformBuffers(), descset, image_infos);

    std::array<VkWriteDescriptorSet, 6> desc_writes{};
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

    std::array<VkDescriptorBufferInfo, 1> light_buffer_infos;
    VkDescriptorBufferInfo light_buffer_info{};
    light_buffer_info.buffer = light_buffer.buffer;
    light_buffer_info.offset = 0;
    light_buffer_info.range = VK_WHOLE_SIZE;
    light_buffer_infos[0] = light_buffer_info;

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

    desc_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[2].dstSet = descset;
    desc_writes[2].dstBinding = 2;
    desc_writes[2].dstArrayElement = 0;
    desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[2].descriptorCount = static_cast<std::uint32_t>(env_image_infos.size());
    desc_writes[2].pImageInfo = env_image_infos.data();

    desc_writes[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[3].dstSet = descset;
    desc_writes[3].dstBinding = 3;
    desc_writes[3].dstArrayElement = 0;
    desc_writes[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    desc_writes[3].descriptorCount = static_cast<std::uint32_t>(light_buffer_infos.size());
    desc_writes[3].pBufferInfo = light_buffer_infos.data();

    desc_writes[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[4].dstSet = descset;
    desc_writes[4].dstBinding = 4;
    desc_writes[4].dstArrayElement = 0;
    desc_writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[4].descriptorCount = static_cast<std::uint32_t>(csm_image_info.size());
    desc_writes[4].pImageInfo = csm_image_info.data();

    desc_writes[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[5].dstSet = descset;
    desc_writes[5].dstBinding = 5;
    desc_writes[5].dstArrayElement = 0;
    desc_writes[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[5].descriptorCount = static_cast<std::uint32_t>(depth_map_image_info.size());
    desc_writes[5].pImageInfo = depth_map_image_info.data();

    vkUpdateDescriptorSets(
      renderer.getAPI()->getDevice(),
      static_cast<std::uint32_t>(desc_writes.size()),
      desc_writes.data(),
      0,
      nullptr);

    mesh.setDescSet(descset);
  }
}
