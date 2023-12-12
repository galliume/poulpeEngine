#include "Skybox.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    struct constants;

    Skybox::Skybox(VulkanAdapter* adapter) :
        m_Adapter(adapter)
    {
     
    }

    void Skybox::visit([[maybe_unused]] float const deltaTime, Mesh* mesh)
    {
        if (!mesh && !mesh->isDirty()) return;

        std::vector<Vertex> const skyVertices = {
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f,  1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},

            {{-1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f, -1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}},
            {{ 1.0f, -1.0f,  1.0f }, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}}
        };

        UniformBufferObject ubo;
        ubo.model = glm::mat4(0.0f);
        //ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        ubo.projection = m_Adapter->getPerspective();

        auto commandPool = m_Adapter->rdr()->createCommandPool();

        Mesh::Data data;
        data.m_Texture = "skybox";
        data.m_Vertices = skyVertices;
        data.m_VertexBuffer = m_Adapter->rdr()->createVertexBuffer(commandPool, skyVertices);
        data.m_Ubos.emplace_back(ubo);
        data.m_TextureIndex = 0;


        vkDestroyCommandPool(m_Adapter->rdr()->getDevice(), commandPool, nullptr);

        Mesh::Buffer uniformBuffer = m_Adapter->rdr()->createUniformBuffers(1);
        mesh->getUniformBuffers()->emplace_back(uniformBuffer);

        mesh->setName("skybox");
        mesh->setShaderName("skybox");

        setPushConstants(mesh);

        for (uint32_t i = 0; i < mesh->getUniformBuffers()->size(); i++) {
            m_Adapter->rdr()->updateUniformBuffer(mesh->getUniformBuffers()->at(i), & data.m_Ubos);
        }

        mesh->setData(data);
        mesh->setIsDirty(false);
    }

    void Skybox::setPushConstants(Mesh* mesh)
    {
        mesh->applyPushConstants = [](VkCommandBuffer & commandBuffer, VkPipelineLayout pipelineLayout,
            VulkanAdapter* adapter, Mesh* mesh) {

            constants pushConstants{};
            pushConstants.textureIDBB = glm::vec3(mesh->getData()->m_TextureIndex, 0.0f, 0.0f);
            pushConstants.view = glm::mat4(glm::mat3(adapter->getCamera()->lookAt()));
            pushConstants.viewPos = adapter->getCamera()->getPos();

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(constants),
                & pushConstants);
        };

        mesh->setHasPushConstants();
    }
}
