#include "rebulkpch.h"
#include "TextureManager.h"

namespace Rbk
{
    TextureManager::TextureManager(
        std::shared_ptr<VulkanRenderer> renderer,
        nlohmann::json textureConfig)
        : m_Renderer(renderer), m_TextureConfig(textureConfig)
    {

    }

    std::vector<std::future<void>> TextureManager::Load()
    {
        std::vector<std::future<void>> futures;

        std::future textureFuture = std::async(std::launch::async, [this]() {
            for (auto& texture : m_TextureConfig["textures"].items()) {
                AddTexture(texture.key(), texture.value());
            }
        });
        std::future skyboxFuture = std::async(std::launch::async, [this]() {
            std::vector<std::string>skyboxImages;
            for (auto& texture : m_TextureConfig["skybox"].items()) {
                skyboxImages.emplace_back(texture.value());
            }
            AddSkyBox(skyboxImages);
        });

        futures.emplace_back(std::move(skyboxFuture));
        futures.emplace_back(std::move(textureFuture));

        return futures;
    }

    void TextureManager::AddSkyBox(const std::vector<std::string>& skyboxImages)
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
        VkCommandPool commandPool = m_Renderer->CreateCommandPool();

        texWidth = static_cast<uint32_t>(texWidth);
        texHeight = static_cast<uint32_t>(texHeight);
        texChannels = static_cast<uint32_t>(texChannels);

        VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];
        m_Renderer->BeginCommandBuffer(commandBuffer);
        m_Renderer->CreateSkyboxTextureImage(commandBuffer, skyboxPixels, texWidth, texHeight, mipLevels, skyboxImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->CreateSkyboxImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->CreateSkyboxTextureSampler(mipLevels);

        m_Skybox.SetImage(skyboxImage);
        m_Skybox.SetImageMemory(textureImageMemory);
        m_Skybox.SetImageView(textureImageView);
        m_Skybox.SetSampler(textureSampler);
        m_Skybox.SetMipLevels(mipLevels);
        m_Skybox.SetWidth(texWidth);
        m_Skybox.SetHeight(texHeight);
        m_Skybox.SetChannels(texChannels);

        vkFreeCommandBuffers(m_Renderer->GetDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
    }

    void TextureManager::AddTexture(const std::string& name, const std::string& path)
    {
        if (!std::filesystem::exists(path.c_str())) {
            Rbk::Log::GetLogger()->critical("texture file {} does not exits.", path);
            return;
        }

        if (0 != m_Textures.count(name.c_str())) {
            Rbk::Log::GetLogger()->debug("Texture {} already imported", name);
            return;
        }

        m_Paths.insert({ name, path });

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            Rbk::Log::GetLogger()->warn("failed to load texture image %s", name);
            return;
        }

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        VkCommandPool commandPool = m_Renderer->CreateCommandPool();
        VkCommandBuffer commandBuffer = m_Renderer->AllocateCommandBuffers(commandPool)[0];

        m_Renderer->BeginCommandBuffer(commandBuffer);
        m_Renderer->CreateTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, textureImageMemory, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->CreateTextureSampler(mipLevels);

        Rbk::Texture texture;
        texture.SetName(name);
        texture.SetImage(textureImage);
        texture.SetImageMemory(textureImageMemory);
        texture.SetImageView(textureImageView);
        texture.SetSampler(textureSampler);
        texture.SetMipLevels(mipLevels);
        texture.SetWidth(texWidth);
        texture.SetHeight(texHeight);
        texture.SetChannels(texChannels);

        m_Textures.emplace(name, texture);
        vkFreeCommandBuffers(m_Renderer->GetDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->GetDevice(), commandPool, nullptr);
    }
}
