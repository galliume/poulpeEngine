#include "Grid.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct cPC;

    void Grid::visit([[maybe_unused]] float const deltaTime, IVisitable* const mesh)
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

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        UniformBufferObject ubo{};

        Data gridData;
        gridData.m_Textures.emplace_back("grid");
        gridData.m_TextureIndex = 0;
        gridData.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        gridData.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        gridData.m_Ubos.emplace_back(ubo);
        gridData.m_Indices = indices;
        gridData.m_Vertices = vertices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("grid");
        mesh->setShaderName("grid");
        mesh->getUniformBuffers()->emplace_back(m_Adapter->rdr()->createUniformBuffers(1));
       
        setPushConstants(mesh);

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            //gridData.m_Ubos[i].view = m_Adapter->GetCamera()->LookAt();
            gridData.m_Ubos[i].projection = m_Adapter->getPerspective();

            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), & gridData.m_Ubos);
        }

        createDescriptorSet(mesh);
        mesh->setData(gridData);
        mesh->setIsDirty(false);
    }

    void Grid::setPushConstants(IVisitable* const mesh)
    {
        mesh->setApplyPushConstants([](VkCommandBuffer& commandBuffer,
            VkPipelineLayout pipelineLayout,
            VulkanAdapter* const adapter,
            [[maybe_unused]] IVisitable* const mesh) {
            
            constants pushConstants{};
            pushConstants.view = adapter->getCamera()->lookAt();
            pushConstants.viewPos = glm::vec4(0.1f, 50.f, 0.f, 0.f);

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(constants), & pushConstants);
        });
        mesh->setHasPushConstants();
    }

    void Grid::createDescriptorSet(IVisitable* const mesh)
    {
      Texture ctex = m_TextureManager->getTextures()["mpoulpe"];

      std::vector<VkDescriptorImageInfo> imageInfos{};

      VkDescriptorImageInfo imageInfo{};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = ctex.getImageView();
      imageInfo.sampler = ctex.getSampler();

      imageInfos.emplace_back(imageInfo);

      auto pipeline = m_Adapter->getPipeline(mesh->getShaderName());
      VkDescriptorSet descSet = m_Adapter->rdr()->createDescriptorSets(pipeline->descPool, { pipeline->descSetLayout }, 1);

      m_Adapter->rdr()->updateDescriptorSets(*mesh->getUniformBuffers(), descSet, imageInfos);

      mesh->setDescSet(descSet);
    }
}
