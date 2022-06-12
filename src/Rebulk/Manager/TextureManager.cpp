#include "rebulkpch.h"
#include "TextureManager.h"

namespace Rbk
{
    TextureManager::TextureManager(std::shared_ptr<VulkanRenderer> renderer) : m_Renderer(renderer)
    {

    }

    void TextureManager::AddSkyBox(std::vector<std::string> skyboxImages)
    {	
        int texWidth = 0, texHeight = 0, texChannels = 0;
        std::vector<stbi_uc*>skyboxPixels;

        for (std::string path : skyboxImages) {
            if (!std::filesystem::exists(path)) {
                Rbk::Log::GetLogger()->critical("texture file {} does not exits.", path);
                return;
            }

            stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

            if (!pixels) {
                Rbk::Log::GetLogger()->warn("failed to load skybox texture image %s", path);
                return;
            }

            skyboxPixels.emplace_back(pixels);
        }

        VkImage skyboxImage;
        VkDeviceMemory textureImageMemory;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        VkCommandPool commandPool = m_Renderer.get()->CreateCommandPool();

        texWidth = static_cast<uint32_t>(texWidth);
        texHeight = static_cast<uint32_t>(texHeight);
        texChannels = static_cast<uint32_t>(texChannels);

        VkCommandBuffer commandBuffer = m_Renderer.get()->AllocateCommandBuffers(commandPool)[0];
        m_Renderer.get()->BeginCommandBuffer(commandBuffer);
        m_Renderer.get()->CreateSkyboxTextureImage(commandBuffer, skyboxPixels, texWidth, texHeight, mipLevels, skyboxImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer.get()->CreateSkyboxImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer.get()->CreateSkyboxTextureSampler(mipLevels);

        m_Skybox.image = skyboxImage;
        m_Skybox.imageMemory = textureImageMemory;
        m_Skybox.imageView = textureImageView;
        m_Skybox.sampler = textureSampler;
        m_Skybox.mipLevels = mipLevels;
        m_Skybox.width = texWidth;
        m_Skybox.height = texHeight;
        m_Skybox.channels = texChannels;

        vkDestroyCommandPool(m_Renderer.get()->GetDevice(), commandPool, nullptr);
    }

    void TextureManager::AddTexture(std::string name, std::string path)
    {
        if (!std::filesystem::exists(path.c_str())) {
            Rbk::Log::GetLogger()->critical("texture file {} does not exits.", path);
            return;
        }

        if (0 != m_Textures.count(name.c_str())) {
            std::cout << "Texture " << name << " already imported" << std::endl;
            return;
        }

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            Rbk::Log::GetLogger()->warn("failed to load texture image %s", name);
            return;
        }

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        VkCommandPool commandPool = m_Renderer.get()->CreateCommandPool();

        VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];
        m_Renderer.get()->BeginCommandBuffer(commandBuffer);
        m_Renderer.get()->CreateTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer.get()->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer.get()->CreateTextureSampler(mipLevels);

        Rbk::Texture texture;
        texture.name = name;
        texture.image = textureImage;
        texture.imageMemory = textureImageMemory;
        texture.imageView = textureImageView;
        texture.sampler = textureSampler;
        texture.mipLevels = mipLevels;
        texture.width = texWidth;
        texture.height = texHeight;
        texture.channels = texChannels;

        m_Textures.emplace(name, texture);
        vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
    }
}
