#include "Crosshair.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    Crosshair::Crosshair(VulkanAdapter* adapter) :
         m_Adapter(adapter)
    {

    }

    void Crosshair::visit([[maybe_unused]] float const deltaTime, Mesh* mesh)
    {
        if (!mesh && !mesh->isDirty()) return;

        const std::vector<Vertex> vertices = {
            {{-0.025f, -0.025f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{0.025f, -0.025f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{0.025f, 0.025f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-0.025f, 0.025f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}
        };
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        UniformBufferObject ubo{};

        Mesh::Data data;
        data.m_Texture = "crosshair";
        data.m_TextureIndex = 0;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, vertices);
        data.m_IndicesBuffer = m_Adapter->rdr()->createIndexBuffer(commandPool, indices);
        data.m_Ubos.emplace_back(ubo);
        data.m_Indices = indices;

        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        mesh->setName("crosshair");
        mesh->setShaderName("2d");
        mesh->getUniformBuffers()->emplace_back(m_Adapter->rdr()->createUniformBuffers(1));

        setPushConstants(mesh);

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            data.m_Ubos[i].projection = m_Adapter->getPerspective();

            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), & data.m_Ubos);
        }

        mesh->setData(data);
        mesh->setIsDirty(false);
    }

    void Crosshair::setPushConstants(Mesh* mesh)
    {
        mesh->applyPushConstants = [](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            [[maybe_unused]] VulkanAdapter* adapter, [[maybe_unused]] Mesh* mesh) {

            float id = static_cast<float>(VulkanAdapter::s_Crosshair);

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(id, 0.0f, 0.0f);

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(constants), & pushConstants);
        };
        mesh->setHasPushConstants();
    }
}
