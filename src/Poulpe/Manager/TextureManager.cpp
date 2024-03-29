#include "TextureManager.hpp"

#include <filesystem>

namespace Poulpe
{
  std::vector<std::array<float, 3>> TextureManager::addNormalMapTexture(std::string const& name)
  {
    if (!m_Textures.contains(name)) {
      PLP_TRACE("Texture {} does not exists, can't create normal map", name);
      return {};
    }

    Texture& originalTexture = m_Textures[name];
    auto const path = originalTexture.getPath();

    if (!originalTexture.getNormalMap().empty()) {
      return originalTexture.getNormalMap();
    }

    int texWidth = 0, texHeight = 0, texChannels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels) {
      PLP_FATAL("failed to load texture image %s", name);
      return {};
    }

    std::vector<std::array<float, 3>> normalMapTexture;

    for (int y = 0; y < texHeight; ++y) {

      int ym1 = (y - 1) & (texHeight - 1);
      int yp1 = (y + 1) & (texHeight - 1);

      unsigned char* centerRow = pixels + y * texWidth;
      unsigned char* upperRow = pixels + ym1 * texWidth;
      unsigned char* lowerRow = pixels + yp1 * texWidth;

      for (int x = 0; x < texWidth; ++x) {
        int xm1 = (x - 1) & (texWidth - 1);
        int xp1 = (x + 1) & (texWidth - 1);

        float dx = (centerRow[xp1] - centerRow[xm1]) * 0.5f;
        float dy = (lowerRow[x] - upperRow[x]) * 0.5f;

        float nz = 1.0f / std::sqrt(dx * dx + dy * dy + 1.0f);
        float nx = std::fmin(std::fmax(-dx * nz, -1.0f), 1.0f);
        float ny = std::fmin(std::fmax(-dy * nz, -1.0f), 1.0f);

        std::array<float, 3>d{ nx, ny, nz };
        normalMapTexture.emplace_back(d);
      }
    }

    originalTexture.setNormalMap(std::move(normalMapTexture));

    return originalTexture.getNormalMap();
  }

  void TextureManager::addSkyBox(std::vector<std::string> const& skyboxImages)
  {
    m_Skybox = {};

    int texWidth = 0, texHeight = 0, texChannels = 0;
    std::vector<stbi_uc*>skyboxPixels;

    for (std::string path : skyboxImages) {
      if (!std::filesystem::exists(path)) {
        PLP_FATAL("texture file {} does not exits.", path);
        return;
      }

      stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

      if (!pixels) {
        PLP_FATAL("failed to load skybox texture image %s", path);
        return;
      }

      skyboxPixels.emplace_back(pixels);
    }

    VkImage skyboxImage = nullptr;
    uint32_t mipLevels = 1;
    VkCommandPool commandPool = m_Renderer->createCommandPool();

    VkCommandBuffer commandBuffer = m_Renderer->allocateCommandBuffers(commandPool)[0];
    m_Renderer->beginCommandBuffer(commandBuffer);
    m_Renderer->createSkyboxTextureImage(commandBuffer,
      skyboxPixels,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      mipLevels,
      skyboxImage,
      VK_FORMAT_R8G8B8A8_SRGB);

    VkImageView textureImageView = m_Renderer->createSkyboxImageView(skyboxImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
    VkSampler textureSampler = m_Renderer->createSkyboxTextureSampler(mipLevels);

    m_Skybox.setImage(skyboxImage);
    m_Skybox.setImageView(textureImageView);
    m_Skybox.setSampler(textureSampler);
    m_Skybox.setMipLevels(mipLevels);
    m_Skybox.setWidth(static_cast<uint32_t>(texWidth));
    m_Skybox.setHeight(static_cast<uint32_t>(texHeight));
    m_Skybox.setChannels(static_cast<uint32_t>(texChannels));
    m_Skybox.setIsPublic(true);

    vkFreeCommandBuffers(m_Renderer->getDevice(), commandPool, 1, &commandBuffer);
    vkDestroyCommandPool(m_Renderer->getDevice(), commandPool, nullptr);
  }

  void TextureManager::addTexture(std::string const& name, std::string const& path, bool isPublic)
  {
    if (!std::filesystem::exists(path.c_str())) {
      PLP_FATAL("texture file {} does not exits.", path);
      //return;
    }

    if (0 != m_Textures.count(name.c_str())) {
      PLP_TRACE("Texture {} already imported", name);
      return;
    }

    m_Paths.insert({ name, path });

    int texWidth = 0, texHeight = 0, texChannels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels) {
      PLP_FATAL("failed to load texture image %s", name);
      return;
    }

    VkImage textureImage = nullptr;
    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    if (std::cmp_greater(mipLevels, MAX_MIPLEVELS)) mipLevels = MAX_MIPLEVELS;

    VkCommandPool commandPool = m_Renderer->createCommandPool();
    VkCommandBuffer commandBuffer = m_Renderer->allocateCommandBuffers(commandPool)[0];

    m_Renderer->beginCommandBuffer(commandBuffer);
    m_Renderer->createTextureImage(commandBuffer,
      pixels,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      mipLevels,
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB);

    VkImageView textureImageView = m_Renderer->createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, mipLevels);
    VkSampler textureSampler = m_Renderer->createTextureSampler(mipLevels);

    Texture texture;
    texture.setName(name);
    texture.setImage(textureImage);
    texture.setImageView(textureImageView);
    texture.setSampler(textureSampler);
    texture.setMipLevels(mipLevels);
    texture.setWidth(static_cast<uint32_t>(texWidth));
    texture.setHeight(static_cast<uint32_t>(texHeight));
    texture.setChannels(static_cast<uint32_t>(texChannels));
    texture.setIsPublic(isPublic);
    texture.setPath(path);

    m_Textures.emplace(name, texture);
    vkFreeCommandBuffers(m_Renderer->getDevice(), commandPool, 1, &commandBuffer);
    vkDestroyCommandPool(m_Renderer->getDevice(), commandPool, nullptr);
  }

  void TextureManager::clear()
  {
    m_Textures.clear();
    m_TexturesLoadingDone = false;
    m_SkyboxLoadingDone = false;
    m_TextureConfig.clear();
  }

  std::function<void()> TextureManager::load()
  {
    std::function<void()> textureFuture = [this]() {
      std::filesystem::path p = std::filesystem::current_path();

      for (auto& [key, path] : m_TextureConfig["textures"].items()) {
        auto absolutePath = p.string() + "/" + static_cast<std::string>(path);
        addTexture(key, absolutePath, true);
      }

      m_TexturesLoadingDone.store(true);
    };

    return textureFuture;
  }

  std::function<void()> TextureManager::loadSkybox(std::string_view skybox)
  {
    m_SkyboxName = skybox;
    std::function<void()> skyboxFuture = [this]() {
      std::vector<std::string>skyboxImages;
      for (auto& texture : m_TextureConfig["skybox"][m_SkyboxName].items()) {
        skyboxImages.emplace_back(texture.value());
      }
      addSkyBox(skyboxImages);
      m_SkyboxLoadingDone.store(true);
    };

    return skyboxFuture;
  }
}
