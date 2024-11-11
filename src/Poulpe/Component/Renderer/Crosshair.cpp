#include "Crosshair.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

namespace Poulpe
{
    void Crosshair::createDescriptorSet(Mesh* mesh)
    {
      Texture const tex{ _texture_manager->getTextures()["crosshair_1"] };
      Texture const tex2{ _texture_manager->getTextures()["crosshair_2"] };

      std::vector<VkDescriptorImageInfo> imageInfos{};
      imageInfos.reserve(2);
      imageInfos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(tex2.getSampler(), tex2.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      auto const& pipeline = _renderer->getPipeline(mesh->getShaderName());

      VkDescriptorSet descSet = _renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      _renderer->updateDescriptorSets(*mesh->getUniformBuffers(), descSet, imageInfos);
      mesh->setDescSet(descSet);
    }

    void Crosshair::setPushConstants(Mesh* mesh)
    {
        mesh->setApplyPushConstants([](
            VkCommandBuffer & cmd_buffer, VkPipelineLayout pipelineLayout,
             Renderer* const renderer,
             Mesh* const meshCH) {
  
            float id = 0.0f;

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(id, 0.0f, 0.0f);

            vkCmdPushConstants(cmd_buffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), & pushConstants);
        });
        mesh->setHasPushConstants();
    }

    void Crosshair::operator()(std::chrono::duration<float> const& deltaTime, Mesh* mesh)
    {
      if (!mesh && !mesh->isDirty()) return;

      const std::vector<Vertex> vertices {
          {{-0.025f, -0.025f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0, 0.0, 0.0 }},
          {{0.025f, -0.025f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0, 0.0, 0.0 }},
          {{0.025f, 0.025f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0, 0.0, 0.0 }},
          {{-0.025f, 0.025f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0, 0.0, 0.0 }}
      };
      const std::vector<uint32_t> indices {
          0, 1, 2, 2, 3, 0
      };

      auto commandPool = _renderer->createCommandPool();

      UniformBufferObject ubo{};

      Data data;
      data._textures.emplace_back("crosshair");
      data._texture_index = 0;
      data._vertex_buffer = _renderer->createVertexBuffer(commandPool, vertices);
      data._indices_buffer = _renderer->createIndexBuffer(commandPool, indices);
      data._ubos.emplace_back(ubo);
      data._Indices = indices;


      mesh->setName("crosshair");
      mesh->setShaderName("2d");
      mesh->getUniformBuffers()->emplace_back(_renderer->createUniformBuffers(1, commandPool));

      vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);
      setPushConstants(mesh);

      for (uint32_t i{ 0 }; i < mesh->getUniformBuffers()->size(); i++) {
        data._ubos[i].projection = _renderer->getPerspective();

        _renderer->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &data._ubos);
      }

      createDescriptorSet(mesh);
      mesh->setData(data);
      mesh->setIsDirty(false);
    }
}
