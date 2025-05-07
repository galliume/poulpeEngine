#include "Basic.hpp"

namespace Poulpe
{
  struct constants;

  void Basic::createDescriptorSet(Mesh* mesh)
  {
    Texture tex { _texture_manager->getTextures()[mesh->getData()->_textures.at(0)] };
    tex.setSampler(_renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_diffuse_wrap_mode_u,
      mesh->getMaterial().texture_diffuse_wrap_mode_v,
      tex.getMipLevels()));

    Texture alpha { _texture_manager->getTextures()[mesh->getData()->_alpha] };
    alpha.setSampler(_renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_alpha_wrap_mode_u,
      mesh->getMaterial().texture_alpha_wrap_mode_v,
      alpha.getMipLevels()));

    std::string const bump_map_name{ mesh->getData()->_bump_map };
    Texture texture_bump{ _texture_manager->getTextures()[bump_map_name] };
     texture_bump.setSampler(_renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_bump_wrap_mode_u,
      mesh->getMaterial().texture_bump_wrap_mode_v,
      1));

    if (texture_bump.getWidth() == 0) {
      texture_bump = _texture_manager->getTextures()[PLP_EMPTY];
    }

    std::string specular_map_name{ mesh->getData()->_specular_map };
    Texture texture_specular{ _texture_manager->getTextures()[specular_map_name] };
    texture_specular.setSampler(_renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_specular_wrap_mode_u,
    mesh->getMaterial().texture_specular_wrap_mode_v,
    texture_specular.getMipLevels()));

    if (texture_specular.getWidth() == 0) {
      texture_specular = _texture_manager->getTextures()[PLP_EMPTY];
    }

    std::string metal_roughness_map_name{ mesh->getData()->_metal_roughness};
    Texture texture_metal_roughness { _texture_manager->getTextures()[metal_roughness_map_name] };
    texture_metal_roughness.setSampler(_renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_metal_roughness_wrap_mode_u,
    mesh->getMaterial().texture_metal_roughness_wrap_mode_v,
    texture_metal_roughness.getMipLevels()));

    if (texture_metal_roughness.getWidth() == 0) {
      texture_metal_roughness = _texture_manager->getTextures()[PLP_EMPTY];
    }

    std::string emissive_map_name{ mesh->getData()->_emissive};
    Texture texture_emissive { _texture_manager->getTextures()[emissive_map_name] };
    texture_emissive.setSampler(_renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_emissive_wrap_mode_u,
    mesh->getMaterial().texture_emissive_wrap_mode_v,
    texture_emissive.getMipLevels()));

    if (texture_emissive.getWidth() == 0) {
      texture_emissive = _texture_manager->getTextures()[PLP_EMPTY];
    }

    std::string ao_map_name{ mesh->getData()->_ao};
    Texture texture_ao { _texture_manager->getTextures()[ao_map_name] };
    texture_ao.setSampler(_renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_ao_wrap_mode_u,
    mesh->getMaterial().texture_ao_wrap_mode_v,
    texture_ao.getMipLevels()));

    if (texture_ao.getWidth() == 0) {
      texture_ao = _texture_manager->getTextures()[PLP_EMPTY];

    }
 
    std::string base_color_map_name{ mesh->getData()->_base_color};
    Texture texture_base_color { _texture_manager->getTextures()[base_color_map_name] };
    texture_base_color.setSampler(_renderer->getAPI()->createKTXSampler(
    mesh->getMaterial().texture_base_color_wrap_mode_u,
    mesh->getMaterial().texture_base_color_wrap_mode_v,
    texture_base_color.getMipLevels()));

    if (texture_base_color.getWidth() == 0) {
      texture_base_color = _texture_manager->getTextures()[PLP_EMPTY];
    }

    //VkDescriptorImageInfo shadowMapSpot{};
    //shadowMapSpot.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //shadowMapSpot.imageview = _renderer->getDepthMapImageViews()->at(1);
    //shadowMapSpot.sampler = _renderer->getDepthMapSamplers()->at(1);
    std::vector<VkDescriptorImageInfo> image_info{};
    image_info.reserve(7);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(alpha.getSampler(), alpha.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(texture_bump.getSampler(), texture_bump.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(texture_specular.getSampler(), texture_specular.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(texture_metal_roughness.getSampler(), texture_metal_roughness.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(texture_emissive.getSampler(), texture_emissive.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(texture_ao.getSampler(), texture_ao.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_info.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> cubemap_info{};

    Texture texture_environment{ _texture_manager->getSkyboxTexture() };
    texture_environment.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      0));

    if (texture_environment.getWidth() == 0) {
      texture_environment = _texture_manager->getTextures()[PLP_EMPTY];
    }
    cubemap_info.emplace_back(texture_environment.getSampler(), texture_environment.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkDescriptorImageInfo> depth_map_image_info{};
    depth_map_image_info.emplace_back(_renderer->getDepthMapSamplers(), _renderer->getDepthMapImageViews(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //image_info.emplace_back(shadowMapSpot);

    auto const& pipeline = _renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset{ _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1) };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {

      _renderer->getAPI()->updateDescriptorSets(
        *mesh->getUniformBuffers(),
        *mesh->getObjectStorageBuffer(),
        *mesh->getBonesStorageBuffer(),
        descset, image_info, depth_map_image_info, cubemap_info);
    }

    mesh->setDescSet(descset);

    std::array<VkWriteDescriptorSet, 2> descset_writes{};
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorBufferInfo> buffer_storage_infos;

    std::for_each(std::begin(*mesh->getUniformBuffers()), std::end(*mesh->getUniformBuffers()),
      [&buffer_infos](const Buffer& uniformBuffer)
      {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniformBuffer.buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        buffer_infos.emplace_back(buffer_info);
      });

    auto const& shadow_map_pipeline = _renderer->getPipeline("shadowMap");
    VkDescriptorSet shadow_map_descset = _renderer->getAPI()->createDescriptorSets(shadow_map_pipeline->desc_pool, { shadow_map_pipeline->descset_layout }, 1);

    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = mesh->getObjectStorageBuffer()->buffer;
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
    descset_writes[1].descriptorCount = static_cast<uint32_t>(buffer_storage_infos.size());
    descset_writes[1].pBufferInfo = buffer_storage_infos.data();

    vkUpdateDescriptorSets(_renderer->getDevice(), static_cast<uint32_t>(descset_writes.size()), descset_writes.data(), 0, nullptr);

    mesh->setShadowMapDescSet(shadow_map_descset);
  }

  void Basic::setPushConstants(Mesh* mesh)
  {
    mesh->setApplyPushConstants([](
      VkCommandBuffer & cmd_buffer,
      VkPipelineLayout pipeline_layout,
      Renderer* const renderer, Mesh* const meshB) {

      constants pushConstants{};
      pushConstants.view = renderer->getCamera()->lookAt();
      pushConstants.view_position = renderer->getCamera()->getPos();

      vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
        &pushConstants);
    });

    mesh->setHasPushConstants();
  }

  void Basic::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;

    auto cmd_pool = _renderer->getAPI()->createCommandPool();

    if (mesh->getUniformBuffers()->empty()) {
      std::ranges::for_each(mesh->getData()->_bones, [&](auto const& bone) {
        
        auto const& b{ bone.second };

        Buffer uniformBuffer = _renderer->getAPI()->createUniformBuffers(1, cmd_pool);
        mesh->getUniformBuffers()->emplace_back(std::move(uniformBuffer));
      });
    }
    if (mesh->getUniformBuffers()->empty()) { //no bones
      Buffer uniformBuffer = _renderer->getAPI()->createUniformBuffers(1, cmd_pool);
      mesh->getUniformBuffers()->emplace_back(std::move(uniformBuffer));
    }

    auto const& data = mesh->getData();
    data->_texture_index = 0;

    if (data->_vertex_buffer.buffer == VK_NULL_HANDLE) {
      data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(cmd_pool, data->_vertices);
      data->_indices_buffer = _renderer->getAPI()->createIndexBuffer(cmd_pool, data->_indices);
    } else {
      //suppose we have to update data
      {
        data->_vertex_buffer.memory->lock();
        auto *buffer { data->_vertex_buffer.memory->getBuffer(data->_vertex_buffer.index) };
        _renderer->getAPI()->updateVertexBuffer(cmd_pool, data->_vertices, buffer);
        data->_vertex_buffer.memory->unLock();
      }
    }

    for (auto i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& ubo) {
        ubo.projection = _renderer->getPerspective();
      });
    }

    if (mesh->getObjectStorageBuffer()) {

      Material material{};
      material.base_color = mesh->getMaterial().base_color;
      material.ambient = mesh->getMaterial().ambient;
      material.diffuse = mesh->getMaterial().diffuse;
      material.specular = mesh->getMaterial().specular;
      material.transmittance = mesh->getMaterial().transmittance;
      material.mre_factor = mesh->getMaterial().mre_factor;

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

      //PLP_DEBUG("ambient {}",  material.ambient.r);
      //PLP_DEBUG("diffuse {}",  material.diffuse.r);
      //PLP_DEBUG("specular {}",  material.specular.r);
      //PLP_DEBUG("shi_ior_diss {} {}", material.shi_ior_diss.x, material.shi_ior_diss.y);

      ObjectBuffer object_buffer{};
      object_buffer.point_lights[0] = _light_manager->getPointLights().at(0);
      object_buffer.point_lights[1] = _light_manager->getPointLights().at(1);
      object_buffer.spot_light = _light_manager->getSpotLights().at(0);
      object_buffer.sun_light = _light_manager->getSunLight();
      object_buffer.material = material;

      auto object_storage { _renderer->getAPI()->createStorageBuffers<ObjectBuffer>(object_buffer, cmd_pool) };

      mesh->setObjectBuffer(object_buffer);
      mesh->addObjectStorageBuffer(object_storage);

      if (data->_bone_matrices.size() > 0) {
        BonesBuffer bones_buffer{};
        //bones_buffer.bone_matrices = std::move(data->_bone_matrices);
        bones_buffer.bone_matrices = {glm::mat4(2.0f), glm::mat4(4.0f)};

        auto bones_storage{ _renderer->getAPI()->createStorageBuffers<BonesBuffer>(bones_buffer, cmd_pool) };

        mesh->setBonesBuffer(bones_buffer);
        mesh->addBonesStorageBuffer(bones_storage);
      }
    }

    unsigned int min{ 0 };
    unsigned int max{ 0 };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {

      auto& ubos{ mesh->getUniformBuffers()->at(i) };
      auto& ubos_data{ mesh->getData()->_ubos.at(i) };

      _renderer->getAPI()->updateUniformBuffer(ubos, &ubos_data);
    }

    if (*mesh->getDescSet() == NULL) {
      createDescriptorSet(mesh);
      setPushConstants(mesh);
    }
    mesh->setIsDirty(false);

    vkDestroyCommandPool(_renderer->getDevice(), cmd_pool, nullptr);
  }
}
