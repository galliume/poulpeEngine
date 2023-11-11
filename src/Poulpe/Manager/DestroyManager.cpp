#include "DestroyManager.hpp"

namespace Poulpe
{
    DestroyManager::DestroyManager()
    {

    }

    void DestroyManager::setRenderer(VulkanRenderer* renderer)
    {
        m_Renderer = renderer;
    }

    void DestroyManager::cleanEntities(std::vector<Entity*> entities)
    {
        for (auto entity : entities) {
            Mesh* mesh = dynamic_cast<Mesh*>(entity);
            cleanEntity(mesh);
        }
    }

    void DestroyManager::cleanEntities(std::vector<Mesh*> entities)
    {
        for (auto mesh : entities) {
            cleanEntity(mesh);
        }
    }

    void DestroyManager::cleanEntity(Mesh* entity)
    {
        for (auto buffer : *entity->getUniformBuffers()) {
            m_Renderer->destroyBuffer(buffer.buffer);
        }

        m_Renderer->destroyBuffer(entity->getData()->m_VertexBuffer.buffer);
        m_Renderer->destroyBuffer(entity->getData()->m_IndicesBuffer.buffer);
    }

    void DestroyManager::cleanShaders(std::unordered_map<std::string, std::array<VkShaderModule, 2>> shaders)
    {
        for (auto shader : shaders) {
            vkDestroyShaderModule(m_Renderer->getDevice(), shader.second[0], nullptr);
            vkDestroyShaderModule(m_Renderer->getDevice(), shader.second[1], nullptr);
        }
    }

    void DestroyManager::cleanTextures(std::unordered_map<std::string, Texture> textures)
    {
        for (auto texture : textures) {
            cleanTexture(texture.second);
        }
    }

    void  DestroyManager::cleanTexture(Texture textures)
    {
        vkDestroySampler(m_Renderer->getDevice(), textures.getSampler(), nullptr);
        vkDestroyImage(m_Renderer->getDevice(), textures.getImage(), nullptr);
        vkDestroyImageView(m_Renderer->getDevice(), textures.getImageView(), nullptr);
    }

    void DestroyManager::cleanDeviceMemory()
    {
        m_DeviceMemoryPool->clear();
    }
}
