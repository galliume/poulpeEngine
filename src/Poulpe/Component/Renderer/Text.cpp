#include "Text.hpp"

namespace Poulpe
{
  struct constants;

  void Text::createDescriptorSet(Mesh* mesh)
  {
    auto const sampler = _renderer->getAPI()->createKTXSampler(
      TextureWrapMode::CLAMP_TO_EDGE,
      TextureWrapMode::CLAMP_TO_EDGE,
      1);
 
    std::vector<VkDescriptorImageInfo> image_infos{};

    auto ch = _font_manager->get('I');
    image_infos.emplace_back(sampler, ch.texture.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
    VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

    std::array<VkWriteDescriptorSet, 2> desc_writes{};
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

    vkUpdateDescriptorSets(
      _renderer->getAPI()->getDevice(),
      static_cast<uint32_t>(desc_writes.size()),
      desc_writes.data(),
      0,
      nullptr);

    mesh->setDescSet(descset);
  }

  void Text::setPushConstants(Mesh* mesh)
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

  void Text::operator()(double const delta_time, Mesh* mesh)
  {
    if (!mesh && !mesh->isDirty()) return;
  
    std::vector<Vertex> vertices;
    float scale{100.0f};
    float x{1000.0f};
    float y{1000.0f};

    auto ch = _font_manager->get('I');

    float xpos = x + ch.bearing.x * scale;
    float ypos = y - (ch.size.y - ch.bearing.y) * scale;

    float w = ch.size.x * scale;
    float h = ch.size.y * scale;

    Vertex v{
      { xpos, ypos + h, 0.0f},
      {1.0f, 1.0f, 0.0f}, { 0.0f, 0.0f },
      {0.0f, 0.0f, 0.0f, 0.0f},
      { 0.0, 0.0f, 0.0f, 0.0f} };

    Vertex v1{
      { xpos, ypos, 0.0f},
      {1.0f, 1.0f, 0.0f}, { 0.0f, 1.0f },
      {0.0f, 0.0f, 0.0f, 0.0f},
      { 0.0, 0.0f, 0.0f, 0.0f} };

    Vertex v2{
      { xpos + w, ypos, 0.0f},
      {1.0f, 1.0f, 0.0f}, { 1.0f, 1.0f },
      {0.0f, 0.0f, 0.0f, 0.0f},
      { 0.0, 0.0f, 0.0f, 0.0f} };

    Vertex v3{
      { xpos, ypos + h, 0.0f},
      {1.0f, 1.0f, 0.0f}, { 0.0f, 0.0f },
      {0.0f, 0.0f, 0.0f, 0.0f},
      { 0.0, 0.0f, 0.0f, 0.0f} };

    Vertex v4{
      { xpos + w, ypos, 0.0f},
      {1.0f, 1.0f, 0.0f}, { 1.0f, 1.0f },
      {0.0f, 0.0f, 0.0f, 0.0f},
      { 0.0, 0.0f, 0.0f, 0.0f} };

    Vertex v5{
      { xpos + w, ypos + h, 0.0f},
      {1.0f, 1.0f, 0.0f}, { 1.0f, 0.0f },
      {0.0f, 0.0f, 0.0f, 0.0f},
      { 0.0, 0.0f, 0.0f, 0.0f} };

    vertices.emplace_back(v);
    vertices.emplace_back(v1);
    vertices.emplace_back(v2);
    vertices.emplace_back(v3);
    vertices.emplace_back(v4);
    vertices.emplace_back(v5);

    x += (ch.advance >> 6) * scale;

    UniformBufferObject ubo;
    ubo.model = glm::mat4(1.0f);

    ubo.projection = glm::ortho(
      0.0f,
      static_cast<float>(_renderer->getAPI()->getSwapChainExtent().width),
      0.0f,
      static_cast<float>(_renderer->getAPI()->getSwapChainExtent().height));

    auto commandPool = _renderer->getAPI()->createCommandPool();

    auto const& data = mesh->getData();

    data->_vertices = vertices;
    //data->_indices = indices;

    data->_vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
    data->_texture_index = 0;
    data->_ubos.emplace_back(ubo);

    mesh->getData()->_ubos_offset.emplace_back(1);
    mesh->getUniformBuffers()->emplace_back(_renderer->getAPI()->createUniformBuffers(1, commandPool));
    mesh->getMaterial().double_sided = true;
    mesh->getMaterial().alpha_mode = 2.0;//BLEND

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
