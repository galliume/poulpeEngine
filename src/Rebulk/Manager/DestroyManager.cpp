#include "rebulkpch.h"

#include "DestroyManager.h"

namespace Rbk
{
    DestroyManager::DestroyManager()
    {

    }

    void DestroyManager::SetRenderer(std::shared_ptr<VulkanRenderer> renderer)
    {
        m_Renderer = renderer;
    }

    void DestroyManager::CleanEntities(std::vector<std::shared_ptr<Entity>> entities)
    {
        for (std::shared_ptr<Entity> entity : entities) {
            std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
            CleanEntity(mesh);
        }
    }

    void DestroyManager::CleanEntities(std::vector<std::shared_ptr<Mesh>> entities)
    {
        for (std::shared_ptr<Mesh> mesh : entities) {
            CleanEntity(mesh);
        }
    }

    void DestroyManager::CleanEntity(std::shared_ptr<Mesh> entity)
    {
        for (auto buffer : entity->m_UniformBuffers) {
            m_Renderer->DestroyBuffer(buffer.first);
            m_Renderer->DestroyDeviceMemory(buffer.second);
        }

        for (Data data : *entity->GetData()) {
            m_Renderer->DestroyBuffer(data.m_VertexBuffer.buffer);
            //m_Renderer->DestroyDeviceMemory(data.m_VertexBuffer.second);

            m_Renderer->DestroyBuffer(data.m_IndicesBuffer.buffer);
            //m_Renderer->DestroyDeviceMemory(data.m_IndicesBuffer.second);
        }
        m_Renderer->DestroyPipeline(entity->m_GraphicsPipeline);
        vkDestroyPipelineLayout(m_Renderer->GetDevice(), entity->m_PipelineLayout, nullptr);
    }

    void DestroyManager::CleanShaders(std::map<std::string, std::array<VkShaderModule, 2>> shaders)
    {
        for (auto shader : shaders) {
            vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[0], nullptr);
            vkDestroyShaderModule(m_Renderer->GetDevice(), shader.second[1], nullptr);
        }
    }

    void DestroyManager::CleanTextures(std::map<std::string, Texture> textures)
    {
        for (auto texture : textures) {
            CleanTexture(texture.second);
        }
    }

    void  DestroyManager::CleanTexture(Texture textures)
    {
        vkDestroySampler(m_Renderer->GetDevice(), textures.GetSampler(), nullptr);
        vkDestroyImage(m_Renderer->GetDevice(), textures.GetImage(), nullptr);
        m_Renderer->DestroyDeviceMemory(textures.GetImageMemory());
        vkDestroyImageView(m_Renderer->GetDevice(), textures.GetImageView(), nullptr);
    }
}
