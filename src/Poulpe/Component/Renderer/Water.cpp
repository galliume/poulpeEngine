#include "Water.hpp"

namespace Poulpe
{
  struct constants;

  struct Texture_Region
  {
    int lowest_height;
    int optimal_height;
    int heighest_height;
  };

  void Water::createDescriptorSet(Mesh* mesh)
  {
    Texture tex { _texture_manager->getWaterTexture()};
    
    tex.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (tex.getWidth() == 0) {
      tex = _texture_manager->getTextures()["_plp_empty"];
    }

    std::string const normal{ "_water_normal" };
    Texture texture_normal{ _texture_manager->getTextures()[normal] };
     texture_normal.setSampler(_renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_bump_wrap_mode_u,
      mesh->getMaterial().texture_bump_wrap_mode_v,
      1));

    if (texture_normal.getWidth() == 0) {
      texture_normal = _texture_manager->getTextures()["_plp_empty"];
    }

    std::string const normal2{ "_water_normal2" };
    Texture texture_normal2{ _texture_manager->getTextures()[normal2] };
     texture_normal2.setSampler(_renderer->getAPI()->createKTXSampler(
      mesh->getMaterial().texture_bump_wrap_mode_u,
      mesh->getMaterial().texture_bump_wrap_mode_v,
      1));

    if (texture_normal2.getWidth() == 0) {
      texture_normal2 = _texture_manager->getTextures()["_plp_empty"];

    }

    Texture env { _texture_manager->getSkyboxTexture() };
    env.setSampler(_renderer->getAPI()->createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    env.getMipLevels()));

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(_renderer->getDepthSamplers(), _renderer->getDepthImageViews(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(texture_normal.getSampler(), texture_normal.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(texture_normal2.getSampler(), texture_normal2.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(_renderer->getCurrentSampler(), _renderer->getCurrentImageView(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    
    std::vector<VkDescriptorImageInfo> env_image_infos{};
    env_image_infos.emplace_back(env.getSampler(), env.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    //_renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, image_infos);

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
    desc_writes[1].descriptorCount = image_infos.size();
    desc_writes[1].pImageInfo = image_infos.data();

    desc_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[2].dstSet = descset;
    desc_writes[2].dstBinding = 2;
    desc_writes[2].dstArrayElement = 0;
    desc_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[2].descriptorCount = env_image_infos.size();
    desc_writes[2].pImageInfo = env_image_infos.data();

    vkUpdateDescriptorSets(
      _renderer->getAPI()->getDevice(),
      static_cast<uint32_t>(desc_writes.size()),
      desc_writes.data(),
      0,
      nullptr);

    mesh->setDescSet(descset);
  }

  void Water::setPushConstants(Mesh* mesh)
  {
    mesh->setApplyPushConstants([](
      VkCommandBuffer & cmd_buffer,
      VkPipelineLayout pipeline_layout,
      Renderer* const renderer, Mesh* const meshS) {
      
      glm::vec4 options{
        Poulpe::Locator::getConfigManager()->getElapsedTime(),
        0.0f, 0.0f, 0.0f};

      constants constants{};
      constants.view = renderer->getCamera()->lookAt();
      constants.view_position = renderer->getCamera()->getPos();
      constants.total_position = options;

      vkCmdPushConstants(
        cmd_buffer,
        pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT 
        | VK_SHADER_STAGE_FRAGMENT_BIT
        | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        0,
        sizeof(constants),
        &constants);
    });

    mesh->setHasPushConstants();
  }

  void Water::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;

    Texture const& tex { _texture_manager->getTerrainTexture() };
 
    std::vector<Vertex> vertices;
    int const width{ static_cast<int>(tex.getWidth()) };
    int const height{ static_cast<int>(tex.getHeight()) };

    uint32_t const rez{ 50 };
    uint32_t index{ 0 };

    for(auto i = 0; i < rez - 1; i++) {
      for(auto j = 0; j < rez - 1; j++) {

        index = i + j;
        float const y{ 0.0f };

        Vertex v{ 
          { -width/2.0f + width*i/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f}};

        Vertex v2{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f } };

        Vertex v3{ 
          {-width/2.0f + width*i/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f } };

        Vertex v4{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { index, 0.0f, 0.0f, 0.0f } };

        vertices.push_back(v);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
      }
    }

    UniformBufferObject ubo;
    ubo.model = glm::mat4(1.0f);

    ubo.projection = _renderer->getPerspective();

    auto commandPool = _renderer->getAPI()->createCommandPool();

    auto const& data = mesh->getData();

    data->_vertices = vertices;
    //data->_indices = indices;

    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data->_texture_index = 0;
    data->_ubos.emplace_back(ubo);

    mesh->getMaterial().alpha_mode = 2.0;//BLEND

    mesh->getData()->_ubos_offset.emplace_back(1);
    mesh->getUniformBuffers()->emplace_back(_renderer->getAPI()->createUniformBuffers(1, commandPool));

    for (size_t i{ 0 }; i < mesh->getData()->_ubos.size(); ++i) {
      mesh->getData()->_ubos[i].projection = _renderer->getPerspective();
    }

    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);

    unsigned int min{ 0 };
    unsigned int max{ 0 };

    for (size_t i{ 0 }; i < mesh->getUniformBuffers()->size(); ++i) {
      max = mesh->getData()->_ubos_offset.at(i);
      auto ubos = std::vector<UniformBufferObject>(mesh->getData()->_ubos.begin() + min, mesh->getData()->_ubos.begin() + max);

      min = max;
      if (ubos.empty()) continue;
      _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &ubos);
    }

    createDescriptorSet(mesh);
    setPushConstants(mesh);
    mesh->setIsDirty(false);
  }
}
