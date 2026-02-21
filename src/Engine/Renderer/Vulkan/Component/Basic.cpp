module Engine.Renderer.Vulkan.Basic;

import std;

import Engine.Component.Mesh;

import Engine.Core.Constants;
import Engine.Core.Logger;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Volk;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  void Basic::operator()(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {
    stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;


    if (!mesh.isDirty()) return;

    auto cmd_pool = renderer.getAPI().createCommandPool();

    // if (mesh.getUniformBuffers().empty()) {
    //   std::ranges::for_each(mesh.getData()->_bones, [&](auto const&) {

    //     //auto const& b{ bone.second };

    //     Buffer uniformBuffer = renderer.getAPI().createUniformBuffers(1, renderer.getCurrentFrameIndex());
    //     mesh.getUniformBuffers().emplace_back(std::move(uniformBuffer));
    //   });
    // }
    if (mesh.getUniformBuffers().empty()) { //no bones
      Buffer uniformBuffer = renderer.getAPI().createUniformBuffers(1, renderer.getCurrentFrameIndex());
      mesh.getUniformBuffers().emplace_back(std::move(uniformBuffer));
    }

    auto const& data = mesh.getData();
    data->_texture_index = 0;

    if (data->_vertex_buffer.buffer == VK_NULL_HANDLE) {
      data->_vertex_buffer = renderer.getAPI().createVertexBuffer(data->_vertices, renderer.getCurrentFrameIndex());
      
      if (!data->_indices.empty()) {
        data->_indices_buffer = renderer.getAPI().createIndexBuffer(data->_indices, renderer.getCurrentFrameIndex());
      }
    } else {
      //suppose we have to update data
      {
        data->_is_dirty = true;
        //data->_vertex_buffer.memory->lock();
        //auto *buffer { data->_vertex_buffer.memory->getBuffer(data->_vertex_buffer.index) };
        renderer.updateVertexBuffer(data, renderer.getCurrentFrameIndex());
        //data->_vertex_buffer.memory->unLock();
      }
    }

    for (std::size_t i{ 0 }; i < mesh.getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh.getData()->_ubos.at(i), [&](auto& ubo) {
        ubo.projection = renderer.getPerspective();
      });
    }

    auto const& mat { mesh.getMaterials().at(data->_material_id) };

    if (mesh.getStorageBuffers()->empty()) {
      Material material{};
      material.base_color = mat.base_color;
      material.ambient = mat.ambient;
      material.diffuse = mat.diffuse;
      material.specular = mat.specular;
      material.transmittance = mat.transmittance;
      material.mre_factor = mat.mre_factor;
      material.emissive_color = mat.emissive_color;

      material.shi_ior_diss = glm::vec3(
        mat.shininess,
        mat.ior,
        mat.dissolve);

      material.alpha = glm::vec3(mat.alpha_mode, mat.alpha_cut_off, 1.0);

      //@todo needed to modify assimp sources glTF2Asset.h textureTransformSupported = true (preview ?)
      material.ambient_translation = mat.ambient_translation;
      material.ambient_scale = mat.ambient_scale;
      material.ambient_rotation = { mat.ambient_rotation.x, mat.ambient_rotation.y, 1.0 };

      material.normal_translation = mat.normal_translation;
      material.normal_scale = mat.normal_scale;
      material.normal_rotation = { mat.normal_rotation.x, mat.normal_rotation.y, 1.0 };

      material.diffuse_translation = mat.diffuse_translation;
      material.diffuse_scale = mat.diffuse_scale;
      material.diffuse_rotation = { mat.diffuse_rotation.x, mat.diffuse_rotation.y, 1.0 };

      material.emissive_translation = mat.emissive_translation;
      material.emissive_scale = mat.emissive_scale;
      material.emissive_rotation = { mat.emissive_rotation.x, mat.emissive_rotation.y, 1.0 };

      material.mr_translation = mat.mr_translation;
      material.mr_scale = mat.mr_scale;
      material.mr_rotation = { mat.mr_rotation.x, mat.mr_rotation.y, 1.0 };

      material.strength = { mat.normal_strength, mat.occlusion_strength, 0.0 };//x normal strength, y occlusion strength

      //Logger::debug("ambient {}",  material.ambient.r);
      //Logger::debug("diffuse {}",  material.diffuse.r);
      //Logger::debug("specular {}",  material.specular.r);
      //Logger::debug("shi_ior_diss {} {}", material.shi_ior_diss.x, material.shi_ior_diss.y);

      ObjectBuffer objectBuffer{};
      objectBuffer.material = material;

      auto storageBuffer{ renderer.getAPI().createStorageBuffers(objectBuffer, renderer.getCurrentFrameIndex()) };

      mesh.setObjectBuffer(objectBuffer);
      mesh.addStorageBuffer(storageBuffer);
      mesh.setHasBufferStorage();

      //renderer.updateStorageBuffer(mesh.getStorageBuffers()->at(0), objectBuffer);
    }

    for (std::size_t i{ 0 }; i < mesh.getUniformBuffers().size(); ++i) {

      auto& ubos{ mesh.getUniformBuffers().at(i) };
      auto& ubos_data{ mesh.getData()->_ubos.at(i) };

      renderer.getAPI().updateUniformBuffer(ubos, &ubos_data, renderer.getCurrentFrameIndex());
    }

    if (*mesh.getDescSet() == nullptr) {
      createDescriptorSet(renderer, mesh, render_context);
    }
    mesh.setIsDirty(false);

    vkDestroyCommandPool(renderer.getDevice(), cmd_pool, nullptr);
  }

  void Basic::createDescriptorSet(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context)
  {
    auto const& mat { mesh.getMaterials().at(mesh.getData()->_material_id) };

    auto const & main_texture_name {
      (!mesh.getData()->_base_color.empty()
      && mesh.getData()->_base_color != PLP_EMPTY)
        ? mesh.getData()->_base_color
        : mesh.getData()->_textures.at(0)
    };

    Texture tex { getTexture(render_context, main_texture_name) };
    tex.setSampler(renderer.getAPI().createKTXSampler(
      mat.texture_diffuse_wrap_mode_u,
      mat.texture_diffuse_wrap_mode_v,
      tex.getMipLevels()));

    Texture alpha { getTexture(render_context, mesh.getData()->_alpha) };
    alpha.setSampler(renderer.getAPI().createKTXSampler(
      mat.texture_alpha_wrap_mode_u,
      mat.texture_alpha_wrap_mode_v,
      alpha.getMipLevels()));

    Texture texture_bump{ getTexture(render_context, mesh.getData()->_bump_map) };
      texture_bump.setSampler(renderer.getAPI().createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      1));

    if (texture_bump.getWidth() == 0) {
      texture_bump = render_context.textures->at(PLP_EMPTY);
    }

    Texture texture_specular{ getTexture(render_context, mesh.getData()->_specular_map)};
    texture_specular.setSampler(renderer.getAPI().createKTXSampler(
    mat.texture_specular_wrap_mode_u,
    mat.texture_specular_wrap_mode_v,
    texture_specular.getMipLevels()));

    if (texture_specular.getWidth() == 0) {
      texture_specular = render_context.textures->at(PLP_EMPTY);
    }

    Texture texture_metal_roughness { getTexture(render_context, mesh.getData()->_metal_roughness) };
    texture_metal_roughness.setSampler(renderer.getAPI().createKTXSampler(
    mat.texture_metal_roughness_wrap_mode_u,
    mat.texture_metal_roughness_wrap_mode_v,
    texture_metal_roughness.getMipLevels()));

    if (texture_metal_roughness.getWidth() == 0) {
      texture_metal_roughness = render_context.textures->at(PLP_EMPTY);
    }

    Texture texture_emissive { getTexture(render_context, mesh.getData()->_emissive) };
    texture_emissive.setSampler(renderer.getAPI().createKTXSampler(
    mat.texture_emissive_wrap_mode_u,
    mat.texture_emissive_wrap_mode_v,
    texture_emissive.getMipLevels()));

    if (texture_emissive.getWidth() == 0) {
      texture_emissive = render_context.textures->at(PLP_EMPTY);
    }

    Texture texture_ao { getTexture(render_context, mesh.getData()->_ao) };
    texture_ao.setSampler(renderer.getAPI().createKTXSampler(
    mat.texture_ao_wrap_mode_u,
    mat.texture_ao_wrap_mode_v,
    texture_ao.getMipLevels()));

    if (texture_ao.getWidth() == 0) {
      texture_ao = render_context.textures->at(PLP_EMPTY);
    }

    // Texture texture_base_color { getTexture(render_context, mesh.getData()->_base_color) };
    // texture_base_color.setSampler(renderer.getAPI().createKTXSampler(
    // mat.texture_base_color_wrap_mode_u,
    // mat.texture_base_color_wrap_mode_v,
    // texture_base_color.getMipLevels()));

    // if (texture_base_color.getWidth() == 0) {
    //   texture_base_color = render_context.textures->at(PLP_EMPTY);
    // }

    //VkDescriptorImageInfo shadowMapSpot{};
    //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //shadowMapSpot.imageview = renderer.getDepthMapImageViews()->at(1);
    //shadowMapSpot.sampler = renderer.getDepthMapSamplers()->at(1);
    std::vector<VkDescriptorImageInfo> image_info{};
    image_info.reserve(7);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(alpha.getSampler(), alpha.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(texture_bump.getSampler(), texture_bump.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(texture_specular.getSampler(), texture_specular.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(texture_metal_roughness.getSampler(), texture_metal_roughness.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(texture_emissive.getSampler(), texture_emissive.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(texture_ao.getSampler(), texture_ao.getImageView(), VK_IMAGE_LAYOUT_GENERAL);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorImageInfo> env_info{};

    render_context.skybox->setSampler(renderer.getAPI().createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      0));

    env_info.emplace_back(render_context.skybox->getSampler(), render_context.skybox->getImageView(), VK_IMAGE_LAYOUT_GENERAL);

    std::vector<VkDescriptorImageInfo> depth_map_image_info{};
    depth_map_image_info.emplace_back(renderer.getDepthMapSamplers(), renderer.getDepthMapImageViews(), VK_IMAGE_LAYOUT_GENERAL);
    //image_info.emplace_back(shadowMapSpot);

    std::vector<VkDescriptorImageInfo> csm_image_info{};
    csm_image_info.emplace_back(renderer.getCSMSamplers(), renderer.getCSMImageViews(), VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorSet descset{
      renderer.getAPI().createDescriptorSets(renderer.getPipeline(mesh.getShaderName()), 1) };

    auto light_buffer {render_context.light_buffer};

    for (std::size_t i{ 0 }; i < mesh.getUniformBuffers().size(); ++i) {

      renderer.getAPI().updateDescriptorSets(
        mesh.getUniformBuffers(),
        *mesh.getStorageBuffers(),
        descset,
        image_info,
        depth_map_image_info,
        env_info,
        light_buffer,
        csm_image_info);
    }

    mesh.setDescSet(descset);

    std::array<VkWriteDescriptorSet, 2> descset_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorBufferInfo> buffer_storage_infos;

    std::for_each(std::begin(mesh.getUniformBuffers()), std::end(mesh.getUniformBuffers()),
      [&buffer_infos](const Buffer& uniformBuffer)
      {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniformBuffer.buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        buffer_infos.emplace_back(buffer_info);
      });

    VkDescriptorSet shadow_map_descset { 
      renderer.getAPI().createDescriptorSets(renderer.getPipeline("shadow_map"), 1) };

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = light_buffer.buffer;
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;
    buffer_storage_infos.emplace_back(buffer_info);

    descset_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descset_writes[0].dstSet = shadow_map_descset;
    descset_writes[0].dstBinding = 0;
    descset_writes[0].dstArrayElement = 0;
    descset_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descset_writes[0].descriptorCount = 1;
    descset_writes[0].pBufferInfo = buffer_infos.data();

    descset_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descset_writes[1].dstSet = shadow_map_descset;
    descset_writes[1].dstBinding = 1;
    descset_writes[1].dstArrayElement = 0;
    descset_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descset_writes[1].descriptorCount = static_cast<std::uint32_t>(buffer_storage_infos.size());
    descset_writes[1].pBufferInfo = buffer_storage_infos.data();

    vkUpdateDescriptorSets(renderer.getDevice(), static_cast<std::uint32_t>(descset_writes.size()), descset_writes.data(), 0, nullptr);

    mesh.setShadowMapDescSet(shadow_map_descset);

    VkDescriptorSet csm_descset {
      renderer.getAPI().createDescriptorSets(renderer.getPipeline("csm"), 1) };

    std::array<VkWriteDescriptorSet, 2> csm_descset_writes{};
    std::vector<VkDescriptorBufferInfo> csm_buffer_infos;
    std::vector<VkDescriptorBufferInfo> csm_buffer_storage_infos;

    VkDescriptorBufferInfo csm_buffer_info{};
    csm_buffer_info.buffer = light_buffer.buffer;
    csm_buffer_info.offset = 0;
    csm_buffer_info.range = VK_WHOLE_SIZE;
    csm_buffer_storage_infos.emplace_back(csm_buffer_info);

    csm_descset_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    csm_descset_writes[0].dstSet = csm_descset;
    csm_descset_writes[0].dstBinding = 0;
    csm_descset_writes[0].dstArrayElement = 0;
    csm_descset_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    csm_descset_writes[0].descriptorCount = 1;
    csm_descset_writes[0].pBufferInfo = buffer_infos.data();

    csm_descset_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    csm_descset_writes[1].dstSet = csm_descset;
    csm_descset_writes[1].dstBinding = 1;
    csm_descset_writes[1].dstArrayElement = 0;
    csm_descset_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    csm_descset_writes[1].descriptorCount = static_cast<std::uint32_t>(csm_buffer_storage_infos.size());
    csm_descset_writes[1].pBufferInfo = csm_buffer_storage_infos.data();

    vkUpdateDescriptorSets(renderer.getDevice(), static_cast<std::uint32_t>(csm_descset_writes.size()), csm_descset_writes.data(), 0, nullptr);

    mesh.setCSMDescSet(csm_descset);
  }

  Texture const& Basic::getTexture(
    RendererContext const& render_context,
    std::string const& name) const
  {
    auto tex_name { name };
    auto const& it { render_context.textures->find(name) };

    if (it == render_context.textures->end()) {
      //Logger::warn("Cannot load texture : {}", name);
      return render_context.textures->at(PLP_ERROR);
    }

    return it->second;
  }
}
