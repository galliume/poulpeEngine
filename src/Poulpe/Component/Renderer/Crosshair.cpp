#include "Crosshair.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

namespace Poulpe
{
    void Crosshair::createDescriptorSet(Mesh* mesh)
    {
      Texture const tex{ m_TextureManager->getTextures()["crosshair_1"] };
      Texture const tex2{ m_TextureManager->getTextures()["crosshair_2"] };

      std::vector<VkDescriptorImageInfo> imageInfos{};
      imageInfos.reserve(2);
      imageInfos.emplace_back(tex.getSampler(), tex.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      imageInfos.emplace_back(tex2.getSampler(), tex2.getImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      auto const& pipeline = m_Renderer->getPipeline(mesh->getShaderName());

      VkDescriptorSet descSet = m_Renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      m_Renderer->updateDescriptorSets(*mesh->getUniformBuffers(), descSet, imageInfos);
      mesh->setDescSet(descSet);
    }

    void Crosshair::setPushConstants(Mesh* mesh)
    {
        mesh->setApplyPushConstants([](
            VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
             Renderer* const renderer,
             Mesh* const meshCH) {
  
            float id = static_cast<float>(Renderer::s_Crosshair);

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(id, 0.0f, 0.0f);

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), & pushConstants);
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

      auto commandPool = m_Renderer->createCommandPool();

      UniformBufferObject ubo{};

      Data data;
      data.m_Textures.emplace_back("crosshair");
      data.m_TextureIndex = 0;
      data.m_VertexBuffer = m_Renderer->createVertexBuffer(commandPool, vertices);
      data.m_IndicesBuffer = m_Renderer->createIndexBuffer(commandPool, indices);
      data.m_Ubos.emplace_back(ubo);
      data.m_Indices = indices;

      vkDestroyCommandPool(m_Renderer->getDevice(), commandPool, nullptr);

      mesh->setName("crosshair");
      mesh->setShaderName("2d");
      mesh->getUniformBuffers()->emplace_back(m_Renderer->createUniformBuffers(1));

      setPushConstants(mesh);

      for (uint32_t i{ 0 }; i < mesh->getUniformBuffers()->size(); i++) {
        data.m_Ubos[i].projection = m_Renderer->getPerspective();

        m_Renderer->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &data.m_Ubos);
      }

      createDescriptorSet(mesh);
      mesh->setData(data);
      mesh->setIsDirty(false);
    }
}
