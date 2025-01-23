#include "Terrain.hpp"

namespace Poulpe
{
  struct constants;

  void Terrain::createDescriptorSet(Mesh* mesh)
  {
    Texture tex { _texture_manager->getTerrainTexture() };
    
    tex.setSampler(_renderer->getAPI()->createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      tex.getMipLevels()));

    if (tex.getWidth() == 0) {
      tex = _texture_manager->getTextures()["_plp_empty"];
    }

    std::vector<VkDescriptorImageInfo> image_infos{};
    image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(constants),
        &pushConstants);
    });

    mesh->setHasPushConstants();
  }

  void Terrain::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;
  
    std::vector<Vertex> vertices;
    unsigned int const width{ 32 };
    unsigned int const height{ 32 };
    float const scale{ 4.0f };

    for(unsigned int i = 0; i < height; i++) {
      for(unsigned int j = 0; j < width; j++) {
        auto const x{ j };
        auto const z{ i };

        glm::vec2 const uv{ static_cast<float>(j) / (width - 1),
                      static_cast<float>(i) / (height - 1) };

        Vertex v{ {x, 1.0, z}, {1.0f, 0.0f, 0.0f}, uv, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} };
        vertices.push_back(v);
      }
    }
    std::vector<uint32_t> indices{};
    for(unsigned int i = 0; i < height - 1; i++) {
      for(unsigned int j = 0; j < width - 1; j++) {
        uint32_t bottom_left = i * width + j;
        uint32_t top_left = (i + 1) * width + j;
        uint32_t top_right = (i + 1) * width + j + 1;
        uint32_t bottom_right = i * width + j + 1;

        indices.push_back(bottom_left);
        indices.push_back(top_left);
        indices.push_back(top_right);

        indices.push_back(bottom_left);
        indices.push_back(top_right);
        indices.push_back(bottom_right);
      }
    }

    UniformBufferObject ubo;
    ubo.model = glm::mat4(1.0f);
    //ubo.model *= glm::mat4_cast(glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 1)));

    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    ubo.projection = _renderer->getPerspective();

    auto commandPool = _renderer->getAPI()->createCommandPool();

    auto const& data = mesh->getData();

    data->_vertices = vertices;
    data->_indices = indices;

    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data->_indices_buffer = _renderer->getAPI()->createIndexBuffer(commandPool, indices);
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
