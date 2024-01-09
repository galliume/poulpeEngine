#include "Grid.hpp"

namespace Poulpe
{
    struct cPC;

    void Grid::createDescriptorSet(IVisitable* const mesh)
    {
      Texture ctex = m_TextureManager->getTextures()["mpoulpe"];

      std::vector<VkDescriptorImageInfo> imageInfos{};

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = ctex.getImageView();
      imageInfo.sampler = ctex.getSampler();

      imageInfos.emplace_back(imageInfo);

      auto pipeline = m_Renderer->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet = m_Renderer->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      m_Renderer->updateDescriptorSets(*mesh->getUniformBuffers(), descSet, imageInfos);

      mesh->setDescSet(descSet);
    }

    void Grid::setPushConstants(IVisitable* const mesh)
    {
        mesh->setApplyPushConstants([](VkCommandBuffer& commandBuffer,
            VkPipelineLayout pipelineLayout,
            IRenderer* const renderer,
            [[maybe_unused]] IVisitable* const meshG) {
            
            constants pushConstants{};
            pushConstants.view = renderer->getCamera()->lookAt();
            pushConstants.viewPos = glm::vec4(0.1f, 50.f, 0.f, 0.f);

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), & pushConstants);
        });
        mesh->setHasPushConstants();
    }

    void Grid::visit([[maybe_unused]] std::chrono::duration<float> deltaTime, IVisitable* const mesh)
    {
      if (!mesh && !mesh->isDirty()) return;

      std::vector<Vertex> const vertices = {
          {{-1.f, -1.f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
          {{1.f, -1.f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
          {{1.0f, 1.f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
          {{-1.f, 1.f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
      };
      std::vector<uint32_t> const indices = {
          0, 1, 2, 2, 3, 0
      };

      auto commandPool = m_Renderer->createCommandPool();

      UniformBufferObject ubo{};

      Data gridData;
      gridData.m_Textures.emplace_back("grid");
      gridData.m_TextureIndex = 0;
      gridData.m_VertexBuffer = m_Renderer->createVertexBuffer(commandPool, vertices);
      gridData.m_IndicesBuffer = m_Renderer->createIndexBuffer(commandPool, indices);
      gridData.m_Ubos.emplace_back(ubo);
      gridData.m_Indices = indices;
      gridData.m_Vertices = vertices;

      vkDestroyCommandPool(m_Renderer->getDevice(), commandPool, nullptr);

      mesh->setName("grid");
      mesh->setShaderName("grid");
      mesh->getUniformBuffers()->emplace_back(m_Renderer->createUniformBuffers(1));

      setPushConstants(mesh);

      for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
        //gridData.m_Ubos[i].view = m_Renderer->GetCamera()->LookAt();
        gridData.m_Ubos[i].projection = m_Renderer->getPerspective();

        m_Renderer->updateUniformBuffer(mesh->getUniformBuffers()->at(i), &gridData.m_Ubos);
      }

      createDescriptorSet(mesh);
      mesh->setData(gridData);
      mesh->setIsDirty(false);
    }
}
