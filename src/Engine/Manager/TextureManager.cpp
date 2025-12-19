module;
#include <ktx.h>
#include <nlohmann/json.hpp>

#include <stb_image.h>
#include <volk.h>

module Engine.Managers.TextureManager;

import std;

import Engine.Component.Texture;

import Engine.Core.Logger;
import Engine.Core.PlpTypedef;

import Engine.Managers.ConfigManagerLocator;

import Engine.Renderer;

namespace Poulpe
{
  std::vector<std::array<float, 3>> TextureManager::addNormalMapTexture(std::string const& name)
  {
    if (!_textures.contains(name)) {
      Logger::trace("Texture {} does not exists, can't create normal map", name);
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
      Logger::error("failed to load texture image %s", name);
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

  void TextureManager::addSkyBox(
    std::string const& skybox_name,
    std::vector<std::string> const& skybox_images,
    Renderer* const renderer)
  {
    auto const root_path { ConfigManagerLocator::get()->rootPath() };

    std::string path { root_path + "/assets/texture/" + skybox_name + ".ktx2"};
    std::string files {};
    bool is_hdr{false};

    for (auto const& image : skybox_images) {
      std::string const image_path{ root_path + "/" + image };
      std::filesystem::path file_name{ image_path };
      std::string original_name{ file_name.string()};

      if (std::filesystem::exists(file_name.replace_extension("hdr"))) {
        original_name = file_name.string();
        is_hdr = true;
      } else if (std::filesystem::exists(file_name.replace_extension("jpg"))) {
        original_name = file_name.string();
      } else if (std::filesystem::exists(file_name.replace_extension("jpeg"))) {
        original_name = file_name.string();
      } else if (std::filesystem::exists(file_name.replace_extension("png"))) {
        original_name = file_name.string();
      }

      files.append("\"" + original_name + "\" ");
    }

    std::string oetf{ "srgb" };
    std::string options { " --encode uastc --uastc-quality 2 --zstd 11 " };

    //https://github.khronos.org/KTX-Software/libktx/ktx_8h.html#a30cc58c576392303d9a5a54b57ef29b5
    std::string  ktx_format{ "R8G8B8A8_SRGB" }; //diffuse default
    ktx_transcode_fmt_e transcoding { KTX_TTF_BC1_RGB };//diffuse default
    if (is_hdr) {
      options.clear(); // no encoding for hdr
      ktx_format = "R32G32B32A32_SFLOAT";
      transcoding = KTX_TTF_BC1_RGB;
      oetf = "linear";
    }
    if (!std::filesystem::exists(path)) {
      std::string cmd{
        "ktx create  --format " + ktx_format + " --assign-oetf " + oetf + " --convert-oetf " + oetf \
        + " --cubemap " + files + " " + path
      };
      Logger::debug("{}", cmd);
      std::system(cmd.c_str());
    }

    addKTXTexture(skybox_name, path, VK_IMAGE_ASPECT_COLOR_BIT, transcoding, renderer, true);
  }

  void TextureManager::addKTXTexture(
    std::string const& name,
    std::string const& path,
    VkImageAspectFlags const aspect_flags,
    ktx_transcode_fmt_e const transcoding,
    Renderer* const renderer,
    bool const is_public)
  {
    if (!std::filesystem::exists(path.c_str())) {
      Logger::critical("texture file {} does not exits.", path);
      //return;
    }

    if (0 != _textures.count(name.c_str())) {
      Logger::trace("Texture {} already imported", name);
      return;
    }

    _paths.insert({ name, path });

    ktxTexture2 *ktx_texture;
    KTX_error_code result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&ktx_texture);

    if (result != KTX_SUCCESS) {
      Logger::warn("Error while loading KTX file: {} error: {}", path, ktxErrorString(result));
    }

    if (ktxTexture2_NeedsTranscoding(ktx_texture)) {
      result = ktxTexture2_TranscodeBasis(ktx_texture, transcoding, 0);
    }

    if (result != KTX_SUCCESS) {
      Logger::warn("Error while transcoding KTX file: {} error: {}", path, ktxErrorString(result));
    }

    VkCommandPool cmd_pool = renderer->getAPI()->createCommandPool();
    VkCommandBuffer cmd_buffer = renderer->getAPI()->allocateCommandBuffers(cmd_pool)[0];
    VkImage texture_image = nullptr;

    renderer->getAPI()->createKTXImage(cmd_buffer, ktx_texture, texture_image);

    VkImageView texture_imageview = renderer->getAPI()->createKTXImageView(ktx_texture, texture_image, aspect_flags);

    Texture texture;
    texture.setName(name);
    texture.setImage(texture_image);
    texture.setImageView(texture_imageview);
    texture.setMipLevels(ktx_texture->numLevels);
    texture.setWidth(static_cast<uint32_t>(ktx_texture->baseWidth));
    texture.setHeight(static_cast<uint32_t>(ktx_texture->baseHeight));
    texture.setIsPublic(is_public);
    texture.setPath(path);

    _textures.emplace(name, texture);

    vkFreeCommandBuffers(renderer->getDevice(), cmd_pool, 1, &cmd_buffer);
    vkDestroyCommandPool(renderer->getDevice(), cmd_pool, nullptr);
  }

  void TextureManager::clear()
  {
    _textures.clear();
    _texture_config.clear();
  }

  std::function<void(std::latch& count_down)> TextureManager::load(Renderer * const renderer)
  {
    return [this, renderer](std::latch& count_down) {
      for (auto& [key, texture_data] : _texture_config["textures"].items()) {
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::DIFFUSE, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["normal"].items()) {
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::NORMAL, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["mr"].items()) {
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::MR, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["emissive"].items()) {
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::EMISSIVE, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["ao"].items()) {
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::AO, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["transmission"].items()) {
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::EMISSIVE, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["terrain"].items()) {
        setTerrainName(key);
        add(key, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::HEIGHT, renderer);
      }
      for (auto& [key, texture_data] : _texture_config["water"].items()) {
        setWaterName(key);
        add(_water_name, texture_data, VK_IMAGE_ASPECT_COLOR_BIT, TEXTURE_TYPE::DIFFUSE, renderer);
      }
      count_down.count_down();
    };
  }

  void TextureManager::add(
    std::string const& name,
    nlohmann::json const& data,
    VkImageAspectFlags const aspect_flags,
    TEXTURE_TYPE texture_type,
    Renderer* const renderer)
  {
    auto const root_path { ConfigManagerLocator::get()->rootPath() };
    std::string const path{ root_path + "/" + data.at("path").get<std::string>() };
    std::filesystem::path file_name{ path };
    std::string original_name{ file_name.string()};

    //bool has_alpha{ false };
    bool is_hdr{ false };
    std::string oetf{ "srgb" };
    std::string options { " --encode basis-lz --clevel 3 --qlevel 126 " };

    //https://github.khronos.org/KTX-Software/libktx/ktx_8h.html#a30cc58c576392303d9a5a54b57ef29b5
    std::string  ktx_format{ "R8G8B8A8_SRGB" }; //diffuse default
    ktx_transcode_fmt_e transcoding { KTX_TTF_BC1_RGB };//diffuse default

    if (std::filesystem::exists(file_name.replace_extension("hdr"))) {
      original_name = file_name.string();
      is_hdr = true;
    } else if (std::filesystem::exists(file_name.replace_extension("jpg"))) {
      original_name = file_name.string();
    } else if (std::filesystem::exists(file_name.replace_extension("jpeg"))) {
      original_name = file_name.string();
    } else if (std::filesystem::exists(file_name.replace_extension("png"))) {
      original_name = file_name.string();
      //has_alpha = true;
      transcoding = KTX_TTF_BC7_RGBA;
    }

    if (is_hdr) {
      options.clear(); // no encoding for hdr
      ktx_format = "R32G32B32_SFLOAT";
    }

    if (name == PLP_EMPTY) {
      transcoding = KTX_TTF_BC7_RGBA;
    }
    switch (texture_type) {
      case TEXTURE_TYPE::NORMAL:
        oetf = "linear";
        options.append("  --normal-mode ");
        transcoding = KTX_TTF_BC5_RG;
        ktx_format = (is_hdr) ? "R32G32_SFLOAT" : "R8G8_UNORM";
        break;
      case TEXTURE_TYPE::MR:
        oetf = "linear";
        transcoding = KTX_TTF_BC7_RGBA;
        ktx_format = (is_hdr) ? "R32G32B32A32_SFLOAT" : "R8G8B8A8_UNORM";
        break;
      case TEXTURE_TYPE::EMISSIVE:
        //@todo check if correct
        oetf = "srgb";
        transcoding = KTX_TTF_BC7_RGBA;
        ktx_format = (is_hdr) ? "R32G32B32_SFLOAT" : "R8G8B8A8_SRGB";
        if (!is_hdr) {
          options.append(" --assign-primaries bt2020 --convert-primaries bt2020 ");
        }
        break;
      case TEXTURE_TYPE::AO:
      case TEXTURE_TYPE::HEIGHT:
        oetf = "linear";
        transcoding = KTX_TTF_BC4_R ;
        ktx_format = (is_hdr) ? "R32_SFLOAT" : "R8_UNORM";
        break;
      default:
      case TEXTURE_TYPE::DIFFUSE:
        if (!is_hdr) {
          options.append(" --assign-primaries bt2020 --convert-primaries bt2020 ");
        }
        break;
    }

    if (!std::filesystem::exists(path)) {
      std::string cmd{
        "ktx create  --format " + ktx_format + " --assign-oetf " + oetf  \
        + options + " \"" + original_name + "\" \"" + path + "\" "
      };
      Logger::debug("{}", cmd);
      std::system(cmd.c_str());
    }

    addKTXTexture(name, path, aspect_flags, transcoding, renderer, true);
  }

  std::function<void(std::latch& count_down)> TextureManager::loadSkybox(
    std::string_view skybox,
    Renderer* const renderer
  )
  {
    _skybox_name = skybox;
    auto const root_path { ConfigManagerLocator::get()->rootPath() };

    return [this, renderer, root_path](std::latch& count_down) {
      std::vector<std::string>skybox_images;

      for (auto& texture : _texture_config["skybox"][_skybox_name].items()) {
        skybox_images.emplace_back(root_path + "/" + texture.value().get<std::string>());
      }

      addSkyBox(_skybox_name, skybox_images, renderer);
      count_down.arrive_and_wait();
    };
  }
}
