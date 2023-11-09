#include "DestroyManager.hpp"

namespace Poulpe
{
    DestroyManager::DestroyManager()
    {

    }

    void DestroyManager::setRenderer(std::shared_ptr<VulkanRenderer> renderer)
    {
        m_Renderer = renderer;
    }

    void DestroyManager::cleanEntities(std::vector<std::shared_ptr<Entity>> entities)
    {
        for (std::shared_ptr<Entity> entity : entities) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
            cleanEntity(mesh);
        }
    }

    void DestroyManager::cleanEntities(std::vector<std::shared_ptr<Mesh>> entities)
    {
        for (std::shared_ptr<Mesh> mesh : entities) {
            cleanEntity(mesh);
        }
    }

    void DestroyManager::cleanEntity(std::shared_ptr<Mesh> entity)
    {
        for (auto buffer : entity->m_UniformBuffers) {
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
