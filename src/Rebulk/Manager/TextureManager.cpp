#include "rebulkpch.h"
#include "TextureManager.h"

namespace Rbk
{
    TextureManager::TextureManager()
    {

    }

    std::function<void()> TextureManager::Load()
    {
        std::function<void()> textureFuture = [=]() {
            for (auto& texture : m_TextureConfig["textures"].items()) {
                AddTexture(texture.key(), texture.value(), true);
            }
            m_TexturesLoadingDone = true;
        };

        return textureFuture;
    }

    std::function<void()> TextureManager::LoadSkybox(const std::string& skybox)
    {
        m_SkyboxName = skybox;
        std::function<void()> skyboxFuture = [=]() {
            std::vector<std::string>skyboxImages;
            for (auto& texture : m_TextureConfig["skybox"][m_SkyboxName].items()) {
                skyboxImages.emplace_back(texture.value());
            }
            AddSkyBox(skyboxImages);
            m_SkyboxLoadingDone = true;
        };

        return skyboxFuture;
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
        uint32_t mipLevels = 1;
        VkCommandPool commandPool = m_Renderer->Rdr()->CreateCommandPool();

        texWidth = static_cast<uint32_t>(texWidth);
        texHeight = static_cast<uint32_t>(texHeight);
        texChannels = static_cast<uint32_t>(texChannels);

        VkCommandBuffer commandBuffer = m_Renderer->Rdr()->AllocateCommandBuffers(commandPool)[0];
        m_Renderer->Rdr()->BeginCommandBuffer(commandBuffer);
        m_Renderer->Rdr()->CreateSkyboxTextureImage(commandBuffer, skyboxPixels, texWidth, texHeight, mipLevels, skyboxImage, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->Rdr()->CreateSkyboxImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->Rdr()->CreateSkyboxTextureSampler(mipLevels);

        m_Skybox.SetImage(skyboxImage);
        m_Skybox.SetImageView(textureImageView);
        m_Skybox.SetSampler(textureSampler);
        m_Skybox.SetMipLevels(mipLevels);
        m_Skybox.SetWidth(texWidth);
        m_Skybox.SetHeight(texHeight);
        m_Skybox.SetChannels(texChannels);
        m_Skybox.SetIsPublic(true);

        vkFreeCommandBuffers(m_Renderer->Rdr()->GetDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->Rdr()->GetDevice(), commandPool, nullptr);
    }

    void TextureManager::AddTexture(const std::string& name, const std::string& path, bool isPublic)
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
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        if (mipLevels > 5) mipLevels = 5;
        VkCommandPool commandPool = m_Renderer->Rdr()->CreateCommandPool();
        VkCommandBuffer commandBuffer = m_Renderer->Rdr()->AllocateCommandBuffers(commandPool)[0];

        m_Renderer->Rdr()->BeginCommandBuffer(commandBuffer);
        m_Renderer->Rdr()->CreateTextureImage(commandBuffer, pixels, texWidth, texHeight, mipLevels, textureImage, VK_FORMAT_R8G8B8A8_SRGB);

        VkImageView textureImageView = m_Renderer->Rdr()->CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
        VkSampler textureSampler = m_Renderer->Rdr()->CreateTextureSampler(mipLevels);

        Rbk::Texture texture;
        texture.SetName(name);
        texture.SetImage(textureImage);
        texture.SetImageView(textureImageView);
        texture.SetSampler(textureSampler);
        texture.SetMipLevels(mipLevels);
        texture.SetWidth(texWidth);
        texture.SetHeight(texHeight);
        texture.SetChannels(texChannels);
        texture.SetIsPublic(isPublic);

        m_Textures.emplace(name, texture);
        vkFreeCommandBuffers(m_Renderer->Rdr()->GetDevice(), commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_Renderer->Rdr()->GetDevice(), commandPool, nullptr);
    }

    void TextureManager::Clear()
    {
        m_Textures.clear();
        m_TexturesLoadingDone = false;
        m_SkyboxLoadingDone = false;
    }

    TextureManager::~TextureManager()
    {

    }
}
