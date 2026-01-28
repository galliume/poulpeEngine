module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>

#include <volk.h>

module Engine.Managers.FontManager;

import std;

import Engine.Component.Texture;

import Engine.Core.Logger;
import Engine.Core.FontTypes;
import Engine.Core.GLM;

import Engine.Managers.ConfigManagerLocator;

namespace Poulpe
{
  Texture FontManager::load()
  {
    Texture texture{};

    if (FT_Init_FreeType(&_ft))
    {
      Logger::error("FREETYPE: Could not init FreeType Library");
      return texture;
    }

    auto const& config_manager = ConfigManagerLocator::get();
    auto const& app_config { ConfigManagerLocator::get()->appConfig() };

    auto const font{ config_manager->rootPath() + "/" + app_config["font"].get<std::string>()};

    if (FT_New_Face(_ft, font.c_str(), 0, &_face)) {
      Logger::error("FREETYPE: Failed to load font {}", font.c_str());
      return texture;
    }

    FT_CharMap found {nullptr};
    FT_CharMap charmap;

    for (auto n{ 0 }; n < _face->num_charmaps; n++) {
      charmap = _face->charmaps[n];
      if (charmap->platform_id == TT_PLATFORM_MICROSOFT
        && charmap->encoding_id == TT_MS_ID_UNICODE_CS) {
        found = charmap;
        break;
      }
    }

    if (!found) {
      Logger::error("FREETYTPE: Failed to find Unicode charmap");
      return texture;
    }

    if (FT_Set_Charmap(_face, found)) {
      Logger::error("FREETYTPE: Failed to set Unicode charmap");
      return texture;
    }

    FT_Set_Char_Size(_face, FONT_WIDTH, FONT_HEIGHT, FONT_RESOLUTION, FONT_RESOLUTION);

    if (FT_Load_Char(_face, 'X', FT_LOAD_RENDER))
    {
      Logger::error("FREETYTPE: Failed to load Glyph");
      return texture;
    }

    VkCommandPool cmd_pool = _renderer->getAPI()->createCommandPool();
    VkCommandBuffer cmd_buffer = _renderer->getAPI()->allocateCommandBuffers(cmd_pool)[0];

    _characters.resize(static_cast<std::size_t>(_face->num_glyphs));

    //float offset{0.0f};
    float x_offset{ 0.f };
    float y_offset{ 0.f };
    int const max_row_width{ 2048 };
    float max_row_height{ 0.f };

    _atlas_width = max_row_width;

    FT_ULong c;
    FT_UInt glyph_index;
    c = FT_Get_First_Char(_face, &glyph_index);

    //@todo do we need to import all glyphs ?
    while (glyph_index != 0) {

      bool renderable{true};

      glyph_index = FT_Get_Char_Index(_face, c);
      if (FT_Load_Glyph(_face, glyph_index, FT_LOAD_RENDER)) {
        Logger::warn("FREETYTPE: Failed to load Glyph");
        renderable = false;
      }

      if (FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_SDF)) {
        Logger::warn("FREETYTPE: Failed to render Glyph");
        renderable = false;
      }

      if (_face->glyph->bitmap.buffer == nullptr
        || _face->glyph->bitmap.width == 0
        || _face->glyph->bitmap.rows == 0) {
        //Logger::debug("FREETYPE: non-renderable glyph {}", glyph_index);
        renderable = false;
      }

      if (renderable) {
        std::vector<int8_t> r_buffer{};

        auto const glyph_width{ _face->glyph->bitmap.width };
        auto const glyph_height{ _face->glyph->bitmap.rows };
        auto const glyph_pitch{ _face->glyph->bitmap.pitch };

        r_buffer.resize(glyph_width * glyph_height, 0x000);

        auto buffer = _face->glyph->bitmap.buffer;
        std::uint32_t index{ 0 };

        for (std::uint32_t y{ 0 }; y < glyph_height; y++) {
          int8_t const * row_buffer = reinterpret_cast<int8_t const *>(buffer) + y * static_cast<std::uint32_t>(glyph_pitch);
          for (std::uint32_t x{ 0 }; x < glyph_width; x++) {
            int8_t sdf = row_buffer[x];
            r_buffer[index++] = sdf;
          }
        }

        FontCharacter character{
          glyph_index,
          glm::ivec2(glyph_width, glyph_height),
          glm::ivec2(_face->glyph->bitmap_left, _face->glyph->bitmap_top),
          _face->glyph->advance.x,
          glyph_width * glyph_height,
          r_buffer,
          glyph_pitch,
          0,
          0
        };

        max_row_height = std::max(max_row_height, character.size.y);

        if (x_offset + character.size.x >= max_row_width) {
          x_offset = 0;
          y_offset += max_row_height + 1.f;
          _atlas_height += static_cast<std::uint32_t>(max_row_height) + 1;
          max_row_height = 0;
        }

        character.x_offset = static_cast<int>(x_offset);
        character.y_offset = static_cast<int>(y_offset);
        x_offset += character.size.x + 1.f;

        _characters[glyph_index] = character;

        //offset += _face->glyph->bitmap.width;
      }
      c = FT_Get_Next_Char(_face, c, &glyph_index);
    }

    _atlas_height += static_cast<std::uint32_t>(max_row_height) + 50;

    VkImage image = nullptr;
    _renderer->getAPI()->createFontImage(
      cmd_buffer,
      _characters,
      _atlas_width,
      _atlas_height, image,
      _renderer->getCurrentFrameIndex());

    VkImageView imageview = _renderer->getAPI()->createFontImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);

    texture.setName("_plp_font_atlas");
    texture.setImage(image);
    texture.setImageView(imageview);
    texture.setMipLevels(1);
    texture.setWidth(static_cast<std::uint32_t>(x_offset));
    texture.setHeight(static_cast<std::uint32_t>(y_offset));
    texture.setIsPublic(true);

    return texture;
  }

  FontCharacter const& FontManager::get(std::uint32_t const c)
  {
    auto glyph_index = FT_Get_Char_Index(_face, c);

    return _characters[glyph_index];
  }

  FontManager::~FontManager()
  {
    FT_Done_Face(_face);
    FT_Done_FreeType(_ft);
  }
}
