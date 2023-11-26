#include "DestroyManager.hpp"

namespace Poulpe
{
    void DestroyManager::setRenderer(VulkanRenderer* renderer)
    {
        m_Renderer = renderer;
    }

    template<std::derived_from<Entity> T>
    void DestroyManager::cleanEntities(std::vector<std::unique_ptr<T>> & entities)
    {
        for (auto & entity : entities) {
            cleanEntity(entity.get());
        }
    }

    template<std::derived_from<Entity> T>
    void DestroyManager::cleanEntities(std::vector<T*> & entities)
    {
        for (auto entity : entities) {
            cleanEntity(entity);
        }
    }

    template<std::derived_from<Entity> T>
    void DestroyManager::cleanEntity(T* entity)
    {
        for (auto buffer : *entity->getMesh()->getUniformBuffers()) {
            m_Renderer->destroyBuffer(buffer.buffer);
        }

        m_Renderer->destroyBuffer(entity->getMesh()->getData()->m_VertexBuffer.buffer);
        m_Renderer->destroyBuffer(entity->getMesh()->getData()->m_IndicesBuffer.buffer);
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

    template void DestroyManager::cleanEntities<Entity>(std::vector<std::unique_ptr<Entity>> & entities);
    template void DestroyManager::cleanEntities<Entity>(std::vector<Entity*> & entities);
}
