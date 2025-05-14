module Poulpe.Components.Renderer;

struct constants;

void Skybox::createDescriptorSet(Mesh* mesh)
{
  Texture tex{ _texture_manager->getSkyboxTexture() };
  tex.setSampler(_renderer->getAPI()->createKTXSampler(
  TextureWrapMode::CLAMP_TO_EDGE,
  TextureWrapMode::CLAMP_TO_EDGE,
  tex.getMipLevels()));

  std::vector<VkDescriptorImageInfo> image_infos{};
  image_infos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
  VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

  _renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, image_infos);

  mesh->setDescSet(descset);
}

void Skybox::setPushConstants(Mesh* mesh)
{
  mesh->setApplyPushConstants([](VkCommandBuffer& cmd_buffer,
    VkPipelineLayout pipeline_layout,
    Renderer* const renderer,
    Mesh* const meshS) {

    constants pushConstants{};
    pushConstants.view = glm::mat4(glm::mat3(renderer->getCamera()->lookAt()));
    pushConstants.view_position = renderer->getCamera()->getPos();

    vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants),
      &pushConstants);
  });

  mesh->setHasPushConstants();
}

void Skybox::operator()(double delta_time, Mesh* mesh)
{
  if (!mesh && !mesh->isDirty()) return;

  std::vector<Vertex> const vertices {
    {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

    {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

    {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

    {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

    {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },

    {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
    {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
  };

  std::vector<UniformBufferObject> ubos{};
  ubos.reserve(1);

  UniformBufferObject ubo{};
  ubo.model = glm::mat4(0.0f);
  //ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
  ubo.projection = _renderer->getPerspective();
  ubos.push_back({ ubo });
  auto cmd_pool = _renderer->getAPI()->createCommandPool();

  Data data{};
  data._textures.emplace_back("skybox");
  data._vertices = vertices;
  data._vertex_buffer = _renderer->getAPI()->createVertexBuffer(cmd_pool, vertices);
  data._ubos.resize(1);
  data._ubos[0] = ubos;
  data._texture_index = 0;

  Buffer uniform_buffer = _renderer->getAPI()->createUniformBuffers(1, cmd_pool);
  mesh->getUniformBuffers()->emplace_back(uniform_buffer);

  vkDestroyCommandPool(_renderer->getDevice(), cmd_pool, nullptr);

  mesh->setName("skybox");
  mesh->setShaderName("skybox");

  setPushConstants(mesh);

  _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(0), &data._ubos.at(0));

  createDescriptorSet(mesh);
  mesh->setData(data);
  mesh->setIsDirty(false);
}
