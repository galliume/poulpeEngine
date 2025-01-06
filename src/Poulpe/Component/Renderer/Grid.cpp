#include "Grid.hpp"

namespace Poulpe
{
    struct cPC;

    void Grid::createDescriptorSet(Mesh* mesh)
    {
      Texture const ctex{ _texture_manager->getTextures()["mpoulpe"] };

      std::vector<VkDescriptorImageInfo> imageInfos{};
      imageInfos.emplace_back(ctex.getSampler(), ctex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      auto const pipeline = _renderer->getPipeline(mesh->getShaderName());
      VkDescriptorSet descset = _renderer->getAPI()->createDescriptorSets(pipeline->desc_pool, { pipeline->descset_layout }, 1);

      _renderer->getAPI()->updateDescriptorSets(*mesh->getUniformBuffers(), descset, imageInfos);

      mesh->setDescSet(descset);
    }

    void Grid::setPushConstants(Mesh* mesh)
    {
        mesh->setApplyPushConstants([](VkCommandBuffer& cmd_buffer,
            VkPipelineLayout pipeline_layout,
            Renderer* const renderer,
             Mesh* const meshG) {

            constants pushConstants{};
            pushConstants.view = renderer->getCamera()->lookAt();
            pushConstants.view_position = glm::vec4(0.1f, 50.f, 0.f, 0.f);

            vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), & pushConstants);
        });
        mesh->setHasPushConstants();
    }

    void Grid::operator()(double const delta_time, Mesh* mesh)
    {
      if (!mesh && !mesh->isDirty()) return;

      std::vector<Vertex> const vertices {
          {{-1.f, -1.f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
          {{1.f, -1.f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
          {{1.0f, 1.f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} },
          {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }
      };
      std::vector<uint32_t> const indices {
          0, 1, 2, 2, 3, 0
      };

      auto commandPool = _renderer->getAPI()->createCommandPool();

      UniformBufferObject ubo{};

      Data gridData;
      gridData._textures.emplace_back("grid");
      gridData._texture_index = 0;
      gridData._vertex_buffer = _renderer->getAPI()->createVertexBuffer(commandPool, vertices);
      gridData._indices_buffer = _renderer->getAPI()->createIndexBuffer(commandPool, indices);
      gridData._ubos.emplace_back(ubo);
      gridData._indices = indices;
      gridData._vertices = vertices;


      mesh->setName("grid");
      mesh->setShaderName("grid");
      mesh->getUniformBuffers()->emplace_back(_renderer->getAPI()->createUniformBuffers(1, commandPool));

      vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);

      setPushConstants(mesh);

      for (uint32_t i{ 0 }; i < mesh->getUniformBuffers()->size(); i++) {
        //gridData._ubos[i].view = _renderer->GetCamera()->LookAt();
        gridData._ubos[i].projection = _renderer->getPerspective();

        _renderer->getAPI()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &gridData._ubos);
      }

      createDescriptorSet(mesh);
      mesh->setData(gridData);
      mesh->setIsDirty(false);
    }
}
