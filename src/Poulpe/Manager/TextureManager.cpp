#include "TextureManager.hpp"

#include "RenderManager.hpp"

#include <filesystem>

namespace Poulpe
{
  std::vector<std::array<float, 3>> TextureManager::addNormalMapTexture(std::string const& name)
  {
    if (!_textures.contains(name)) {
      PLP_TRACE("Texture {} does not exists, can't create normal map", name);
      return {};
    }

    Texture& original_texture = _textures[name];
    auto const path = original_texture.getPath();

    if (!original_texture.getNormalMap().empty()) {
      return original_texture.getNormalMap();
    }

    int tex_width = 0, tex_height = 0, tex_channels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    if (!pixels) {
      PLP_FATAL("failed to load texture image %s", name);
      return {};
    }

    std::vector<std::array<float, 3>> normal_map;

    for (int y = 0; y < tex_height; ++y) {

      int ym1 = (y - 1) & (tex_height - 1);
      int yp1 = (y + 1) & (tex_height - 1);

      unsigned char* center_row = pixels + y * tex_width;
      unsigned char* upper_row = pixels + ym1 * tex_width;
      unsigned char* lower_row = pixels + yp1 * tex_width;

      for (int x = 0; x < tex_width; ++x) {
        int xm1 = (x - 1) & (tex_width - 1);
        int xp1 = (x + 1) & (tex_width - 1);

        float dx = (center_row[xp1] - center_row[xm1]) * 0.5f;
        float dy = (lower_row[x] - upper_row[x]) * 0.5f;

        float nz = 1.0f / std::sqrt(dx * dx + dy * dy + 1.0f);
        float nx = std::fmin(std::fmax(-dx * nz, -1.0f), 1.0f);
        float ny = std::fmin(std::fmax(-dy * nz, -1.0f), 1.0f);

        std::array<float, 3>d{ nx, ny, nz };
        normal_map.emplace_back(d);
      }
    }

    original_texture.setNormalMap(std::move(normal_map));

    return original_texture.getNormalMap();
  }

  void TextureManager::addSkyBox(std::vector<std::string> const& skybox_images)
  {
    _skybox = {};

    int tex_width = 0, tex_height = 0, tex_channels = 0;
    std::vector<stbi_uc*>skybox_pixels;

    for (std::string path : skybox_images) {
      if (!std::filesystem::exists(path)) {
        PLP_FATAL("texture file {} does not exits.", path);
        return;
      }

      stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

      if (!pixels) {
        PLP_FATAL("failed to load skybox texture image %s", path);
        return;
      }

      skybox_pixels.emplace_back(pixels);
    }

    VkImage skybox_image = nullptr;
    uint32_t mip_lvls = 1;
    VkCommandPool commandPool = _renderer->getAPI()->createCommandPool();

    VkCommandBuffer cmd_buffer = _renderer->getAPI()->allocateCommandBuffers(commandPool)[0];
    _renderer->getAPI()->beginCommandBuffer(cmd_buffer);
    _renderer->getAPI()->createSkyboxTextureImage(
      cmd_buffer,
      skybox_pixels,
      static_cast<uint32_t>(tex_width),
      static_cast<uint32_t>(tex_height),
      mip_lvls,
      skybox_image,
      VK_FORMAT_R8G8B8A8_SRGB);

    VkImageView texture_imageview = _renderer->getAPI()->createCubeImageView(skybox_image, VK_FORMAT_R8G8B8A8_SRGB, mip_lvls);
    VkSampler texture_sampler = _renderer->getAPI()->createCubeTextureSampler(mip_lvls);

    _skybox.setImage(skybox_image);
    _skybox.setImageView(texture_imageview);
    _skybox.setSampler(texture_sampler);
    _skybox.setMipLevels(mip_lvls);
    _skybox.setWidth(static_cast<uint32_t>(tex_width));
    _skybox.setHeight(static_cast<uint32_t>(tex_height));
    _skybox.setChannels(static_cast<uint32_t>(tex_channels));
    _skybox.setIsPublic(true);

    vkFreeCommandBuffers(_renderer->getDevice(), commandPool, 1, &cmd_buffer);
    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);
  }

  void TextureManager::addTexture(std::string const& name, std::string const& path, bool is_public)
  {
    if (!std::filesystem::exists(path.c_str())) {
      PLP_FATAL("texture file {} does not exits.", path);
      //return;
    }

    if (0 != _textures.count(name.c_str())) {
      PLP_TRACE("Texture {} already imported", name);
      return;
    }

    _paths.insert({ name, path });

    int tex_width = 0, tex_height = 0, tex_channels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    if (!pixels) {
      PLP_FATAL("failed to load texture image %s", name);
      return;
    }

    VkImage texture_image = nullptr;
    uint32_t mip_lvls = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;
    if (std::cmp_greater(mip_lvls, MAX_MIPLEVELS)) mip_lvls = MAX_MIPLEVELS;

    VkCommandPool commandPool = _renderer->getAPI()->createCommandPool();
    VkCommandBuffer cmd_buffer = _renderer->getAPI()->allocateCommandBuffers(commandPool)[0];

    _renderer->getAPI()->beginCommandBuffer(cmd_buffer);
    _renderer->getAPI()->createTextureImage(cmd_buffer,
      pixels,
      static_cast<uint32_t>(tex_width),
      static_cast<uint32_t>(tex_height),
      mip_lvls,
      texture_image,
      VK_FORMAT_R8G8B8A8_SRGB);

    VkImageView texture_imageview = _renderer->getAPI()->createImageView(texture_image, VK_FORMAT_R8G8B8A8_SRGB, mip_lvls);
    VkSampler texture_sampler = _renderer->getAPI()->createTextureSampler(mip_lvls);

    Texture texture;
    texture.setName(name);
    texture.setImage(texture_image);
    texture.setImageView(texture_imageview);
    texture.setSampler(texture_sampler);
    texture.setMipLevels(mip_lvls);
    texture.setWidth(static_cast<uint32_t>(tex_width));
    texture.setHeight(static_cast<uint32_t>(tex_height));
    texture.setChannels(static_cast<uint32_t>(tex_channels));
    texture.setIsPublic(is_public);
    texture.setPath(path);

    _textures.emplace(name, texture);

    vkFreeCommandBuffers(_renderer->getDevice(), commandPool, 1, &cmd_buffer);
    vkDestroyCommandPool(_renderer->getDevice(), commandPool, nullptr);
  }

  void TextureManager::clear()
  {
    _textures.clear();
    _texture_config.clear();
  }

  std::function<void(std::latch& count_down)> TextureManager::load()
  {
    return [this](std::latch& count_down) {
      std::filesystem::path p = std::filesystem::current_path();

      for (auto& [key, path] : _texture_config["textures"].items()) {
        auto absolute_path = p.string() + "/" + static_cast<std::string>(path);
        addTexture(key, absolute_path, true);
      }

      count_down.count_down();
    };
  }

  std::function<void(std::latch& count_down)> TextureManager::loadSkybox(std::string_view skybox)
  {
    _skybox_name = skybox;

    return [this](std::latch& count_down) {
      std::vector<std::string>skybox_images;

      for (auto& texture : _texture_config["skybox"][_skybox_name].items()) {
        skybox_images.emplace_back(texture.value());
      }

      addSkyBox(skybox_images);
      count_down.arrive_and_wait();
    };
  }
}
