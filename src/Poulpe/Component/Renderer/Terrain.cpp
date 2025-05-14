module Poulpe.Component.Renderer.Terrain;

import Poulpe.Component.Texture;

import Poulpe.Component.Mesh;
import Poulpe.Core.PlpTypedef;
import Poulpe.Manager.LightManager;
import Poulpe.Manager.TextureManager;
import Poulpe.Renderer.Vulkan.Renderer;

namespace Poulpe
{
  struct Texture_Region
  {
    int lowest_height;
    int optimal_height;
    int heighest_height;
  };
  
  void Terrain::init(
    Renderer* renderer,
    TextureManager* texture_manager,
    LightManager* light_manager)
  {
    _renderer = renderer;
    _texture_manager = texture_manager;
    _light_manager = light_manager;
  }

  void Terrain::createDescriptorSet(Mesh* mesh)
  {
    Texture height_map { _texture_manager->getTerrainTexture()};
    
    height_map.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (height_map.getWidth() == 0) {
      height_map = _texture_manager->getTextures()[PLP_EMPTY];
    }

    //@todo fix this ugly fix. Needs a real asset unique ID
    Texture ground { _texture_manager->getTextures()[PLP_GROUND]};
    
    ground.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (ground.getWidth() == 0) {
      ground = _texture_manager->getTextures()[PLP_EMPTY];
    }

    Texture grass { _texture_manager->getTextures()[PLP_GRASS]};
    
    grass.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (grass.getWidth() == 0) {
      grass = _texture_manager->getTextures()[PLP_EMPTY];
    }

    Texture snow { _texture_manager->getTextures()[PLP_SNOW]};
    
    snow.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (snow.getWidth() == 0) {
      snow = _texture_manager->getTextures()[PLP_EMPTY];
    }

    Texture sand { _texture_manager->getTextures()[PLP_SAND]};

    sand.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (sand.getWidth() == 0) {
      sand = _texture_manager->getTextures()[PLP_EMPTY];
    }

    Texture low_noise { _texture_manager->getTextures()[PLP_LOW_NOISE]};

    low_noise.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (low_noise.getWidth() == 0) {
      low_noise = _texture_manager->getTextures()[PLP_EMPTY];
    }

    Texture hi_noise { _texture_manager->getTextures()[PLP_HI_NOISE]};

    hi_noise.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (hi_noise.getWidth() == 0) {
      hi_noise = _texture_manager->getTextures()[PLP_EMPTY];
    }

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(height_map.getSampler(), height_map.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(ground.getSampler(), ground.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(grass.getSampler(), grass.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(snow.getSampler(), snow.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(sand.getSampler(), sand.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(hi_noise.getSampler(), hi_noise.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(low_noise.getSampler(), low_noise.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    Texture env { _texture_manager->getSkyboxTexture() };
    env.setSampler(_renderer->getAPI()->createKTXSampler(
    TextureWrapMode::CLAMP_TO_EDGE,
    TextureWrapMode::CLAMP_TO_EDGE,
    env.getMipLevels()));

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

  void Terrain::setPushConstants(Mesh* mesh)
  {
    mesh->setApplyPushConstants([](
      VkCommandBuffer & cmd_buffer,
      VkPipelineLayout pipeline_layout,
      Renderer* const renderer, Mesh* const meshS) {

      constants pushConstants{};
      pushConstants.view = renderer->getCamera()->lookAt();
      pushConstants.view_position = renderer->getCamera()->getPos();

      vkCmdPushConstants(
        cmd_buffer,
        pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT 
        | VK_SHADER_STAGE_FRAGMENT_BIT
        | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        0,
        sizeof(constants),
        &pushConstants);
    });

    mesh->setHasPushConstants();
  }

  void Terrain::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;

    Texture const& tex { _texture_manager->getTerrainTexture() };

    std::vector<Vertex> vertices;
    int const width{ static_cast<int>(tex.getWidth()) };
    int const height{ static_cast<int>(tex.getHeight())};

    unsigned int const rez{ 20 };

    for(auto i = 0; i < rez - 1; i++) {
      for(auto j = 0; j < rez - 1; j++) {

        float const y{ 0.0f };

        Vertex v{ 
          { -width/2.0f + width*i/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}};

        Vertex v2{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*j/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, j / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f, 0.0f } };

        Vertex v3{ 
          {-width/2.0f + width*i/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {i / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f, 0.0f } };

        Vertex v4{ 
          {-width/2.0f + width*(i+1)/(float)rez, y, -height/2.0f + height*(j+1)/(float)rez },
          {1.0f, 1.0f, 0.0f}, {(i+1) / (float)rez, (j+1) / (float)rez }, {0.0f, 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f, 0.0f } };

        vertices.push_back(v);
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
      }
    }
    auto const& data = mesh->getData();
    auto commandPool = _renderer->getAPI()->createCommandPool();

    std::vector<UniformBufferObject> ubos{};
    ubos.reserve(1);
    UniformBufferObject ubo;
    ubo.model = glm::mat4(1.0f);
    ubo.projection = _renderer->getPerspective();
    ubos.push_back(ubo);

    data->_vertices = vertices;
    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data->_texture_index = 0;
    data->_ubos.resize(1);
    data->_ubos[0] = ubos;

    mesh->getData()->_ubos_offset.emplace_back(1);
    mesh->getUniformBuffers()->emplace_back(_renderer->getAPI()->createUniformBuffers(1, commandPool));

    for (auto i{ 0 }; i < mesh->getData()->_ubos.size(); i++) {
      std::ranges::for_each(mesh->getData()->_ubos.at(i), [&](auto& ubo) {
        ubo.projection = _renderer->getPerspective();
      });
    }

    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);

    if (!mesh->getData()->_ubos.empty()) {
      _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(0), &mesh->getData()->_ubos.at(0));
    }

    createDescriptorSet(mesh);
    setPushConstants(mesh);
    mesh->setIsDirty(false);
  }
}
