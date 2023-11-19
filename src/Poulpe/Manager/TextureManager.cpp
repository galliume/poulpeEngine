#include "TextureManager.hpp"

#include <filesystem>

namespace Poulpe
{
    std::function<void()> TextureManager::load(std::condition_variable & cv)
    {
        std::function<void()> textureFuture = [this, & cv]() {
            for (auto& texture : m_TextureConfig["textures"].items()) {
                addTexture(texture.key(), texture.value(), true);
            }

            m_TexturesLoadingDone.store(true);
            cv.notify_one();
        };

        return textureFuture;
    }

    std::function<void()> TextureManager::loadSkybox(std::string_view skybox, std::condition_variable & cv)
    {
        m_SkyboxName = skybox;
        std::function<void()> skyboxFuture = [this, & cv]() {
            std::vector<std::string>skyboxImages;
            for (auto& texture : m_TextureConfig["skybox"][m_SkyboxName].items()) {
                skyboxImages.emplace_back(texture.value());
            }
            addSkyBox(skyboxImages);
            m_SkyboxLoadingDone.store(true);
            cv.notify_one();
        };

        return skyboxFuture;
    }

    void TextureManager::addSkyBox(std::vector<std::string> const & skyboxImages)
    {
        m_Skybox = {};

        int texWidth = 0, texHeight = 0, texChannels = 0;
        std::vector<stbi_uc*>skyboxPixels;

        for (std::string path : skyboxImages) {
            if (!std::filesystem::exists(path)) {
                PLP_FATAL("texture file {} does not exits.", path);
                return;
            }

            stbi_uc* pixels = stbi_load(path.c_str(), & texWidth, & texHeight, & texChannels, STBI_rgb_alpha);

            if (!pixels) {
                PLP_FATAL("failed to load skybox texture image %s", path);
                return;
            }

            skyboxPixels.emplace_back(pixels);
        }

        VkImage skyboxImage = nullptr;
        uint32_t mipLevels = 1;
        VkCommandPool commandPool = m_Renderer->rdr()->createCommandPool();

        texWidth = static_cast<uint32_t>(texWidth);
        texHeight = static_cast<uint32_t>(texHeight);
        texChannels = static_cast<uint32_t>(texChannels);

        VkCommandBuffer commandBuffer = m_Renderer->rdr()->allocateCommandBuffers(commandPool)[0];
        m_Renderer->rdr()->beginCommandBuffer(commandBuffer);
        m_Renderer->rdr()->createSkyboxTextureImage(commandBuffer, skyboxPixels, texWidth, texHeight, mipLevels, skyboxImage, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->rdr()->createSkyboxImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->rdr()->createSkyboxTextureSampler(mipLevels);

        m_Skybox.setImage(skyboxImage);
        m_Skybox.setImageView(textureImageView);
        m_Skybox.setSampler(textureSampler);
        m_Skybox.setMipLevels(mipLevels);
        m_Skybox.setWidth(texWidth);
        m_Skybox.setHeight(texHeight);
        m_Skybox.setChannels(texChannels);
        m_Skybox.setIsPublic(true);

        vkFreeCommandBuffers(m_Renderer->rdr()->getDevice(), commandPool, 1, & commandBuffer);
        vkDestroyCommandPool(m_Renderer->rdr()->getDevice(), commandPool, nullptr);
    }

    void TextureManager::addTexture(std::string const & name, std::string const & path, bool isPublic)
    {
        if (!std::filesystem::exists(path.c_str())) {
            PLP_FATAL("texture file {} does not exits.", path);
            return;
        }

        if (0 != m_Textures.count(name.c_str())) {
            PLP_TRACE("Texture {} already imported", name);
            return;
        }

        m_Paths.insert({ name, path });

        int texWidth = 0, texHeight = 0, texChannels = 0;
        stbi_uc* pixels = stbi_load(path.c_str(), & texWidth, & texHeight, & texChannels, STBI_rgb_alpha);

        if (!pixels) {
            PLP_FATAL("failed to load texture image %s", name);
            return;
        }

        VkImage textureImage = nullptr;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        if (std::cmp_greater(mipLevels, MAX_MIPLEVELS)) mipLevels = MAX_MIPLEVELS;

        VkCommandPool commandPool = m_Renderer->rdr()->createCommandPool();
        VkCommandBuffer commandBuffer = m_Renderer->rdr()->allocateCommandBuffers(commandPool)[0];

        m_Renderer->rdr()->beginCommandBuffer(commandBuffer);
        m_Renderer->rdr()->createTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->rdr()->createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->rdr()->createTextureSampler(mipLevels);

        Poulpe::Texture texture;
        texture.setName(name);
        texture.setImage(textureImage);
        texture.setImageView(textureImageView);
        texture.setSampler(textureSampler);
        texture.setMipLevels(mipLevels);
        texture.setWidth(texWidth);
        texture.setHeight(texHeight);
        texture.setChannels(texChannels);
        texture.setIsPublic(isPublic);

        m_Textures.emplace(name, texture);
        vkFreeCommandBuffers(m_Renderer->rdr()->getDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->rdr()->getDevice(), commandPool, nullptr);
    }

    void TextureManager::clear()
    {
        m_Textures.clear();
        m_TexturesLoadingDone = false;
        m_SkyboxLoadingDone = false;
    }
}
