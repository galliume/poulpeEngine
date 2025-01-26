#include "Terrain.hpp"

namespace Poulpe
{
  struct constants;

  struct Texture_Region
  {
    int lowest_height;
    int optimal_height;
    int heighest_height;
  };

  float attenuation(float min, float max, float x)
  {
    float half_range = std::abs(max - min) * 0.5;
    float mid = min + half_range;
    float distance = std::abs(x - mid);

    float att = std::clamp((half_range - distance) / half_range, 0.0f, 1.0f);
  
    return att;
  }

  void Terrain::createDescriptorSet(Mesh* mesh)
  {
    Texture height_map { _texture_manager->getTerrainTexture()};
    
    height_map.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (height_map.getWidth() == 0) {
      height_map = _texture_manager->getTextures()["_plp_empty"];

    }
    Texture ground { _texture_manager->getTextures()["terrain_ground"]};
    
    ground.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (ground.getWidth() == 0) {
      ground = _texture_manager->getTextures()["_plp_empty"];
    }

    Texture grass { _texture_manager->getTextures()["terrain_grass"]};
    
    grass.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (grass.getWidth() == 0) {
      grass = _texture_manager->getTextures()["_plp_empty"];
    }

    Texture snow { _texture_manager->getTextures()["terrain_snow"]};
    
    snow.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (snow.getWidth() == 0) {
      snow = _texture_manager->getTextures()["_plp_empty"];
    }

    Texture sand { _texture_manager->getTextures()["terrain_sand"]};

    sand.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::WRAP,
      TextureWrapMode::WRAP,
      0));

    if (sand.getWidth() == 0) {
      sand = _texture_manager->getTextures()["_plp_empty"];
    }

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(height_map.getSampler(), height_map.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(ground.getSampler(), ground.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(grass.getSampler(), grass.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(snow.getSampler(), snow.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    image_infos.emplace_back(sand.getSampler(), sand.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    _renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, image_infos);

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

    UniformBufferObject ubo;
    ubo.model = glm::mat4(1.0f);

    ubo.projection = _renderer->getPerspective();

    auto commandPool = _renderer->getAPI()->createCommandPool();

    auto const& data = mesh->getData();

    data->_vertices = vertices;
    //data->_indices = indices;
    data->is_quad = true;

    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data->_texture_index = 0;
    data->_ubos.emplace_back(ubo);

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
