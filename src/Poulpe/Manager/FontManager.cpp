#include "FontManager.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

#include <freetype/ttnameid.h>

namespace Poulpe
{
  Texture FontManager::load()
  {
    Texture texture{};

    if (FT_Init_FreeType(&_ft))
    {
      PLP_ERROR("FREETYPE: Could not init FreeType Library");
      return texture;
    }

    auto const font{Poulpe::Locator::getConfigManager()->appConfig()["font"].get<std::string>()};

    if (FT_New_Face(_ft, font.c_str(), 0, &_face))
    {
      PLP_ERROR("FREETYPE: Failed to load font {}", font.c_str());
      return texture;
    }
    
    FT_CharMap found = 0;
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
      PLP_ERROR("FREETYTPE: Failed to find Unicode charmap");
      return texture;
    }

    if (FT_Set_Charmap(_face, found)) {
      PLP_ERROR("FREETYTPE: Failed to set Unicode charmap");
      return texture;
    }

    FT_Set_Char_Size(_face, 0, 16 * 64, 96, 96);

    if (FT_Load_Char(_face, 'X', FT_LOAD_RENDER))
    {
      PLP_ERROR("FREETYTPE: Failed to load Glyph");
      return texture;
    }

    VkCommandPool cmd_pool = _renderer->getAPI()->createCommandPool();
    VkCommandBuffer cmd_buffer = _renderer->getAPI()->allocateCommandBuffers(cmd_pool)[0];
 
    auto offset{0.0f};
    int x_offset{ 0 };
    int y_offset{ 0 };
    int const max_row_width{ 256 };
    int max_row_height{ 0 };
    
    _atlas_width = max_row_width;

    FT_ULong c;
    FT_UInt glyph_index;
    c = FT_Get_First_Char(_face, &glyph_index);

    while (glyph_index != 0) {

      bool renderable{true};

      glyph_index = FT_Get_Char_Index(_face, c);
      if (FT_Load_Glyph(_face, glyph_index, FT_LOAD_RENDER)) {
        PLP_DEBUG("FREETYTPE: Failed to load Glyph");
        renderable = false;
      }

      if (FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL)) {
        PLP_DEBUG("FREETYTPE: Failed to render Glyph");
        renderable = false;
      }

      if (_face->glyph->bitmap.buffer == NULL
        || _face->glyph->bitmap.width == 0
        || _face->glyph->bitmap.rows == 0) {
        PLP_DEBUG("FREETYPE: non-renderable glyph {}", glyph_index);
        renderable = false;
      }

      if (renderable) {
        std::vector<unsigned char> r_buffer{};
      
        unsigned int glyph_width{ 0 };
        unsigned int glyph_height{ 0 };
        int glyph_pitch{ 0 };

        glyph_width = _face->glyph->bitmap.width;
        glyph_height = _face->glyph->bitmap.rows;
        glyph_pitch = _face->glyph->bitmap.pitch;

        r_buffer.resize(glyph_width * glyph_height * 4, 0);

        auto buffer = _face->glyph->bitmap.buffer;
        int index{ 0 };

        for (auto y{ 0 }; y < glyph_height; y++) {
          const uint8_t* row_buffer = buffer + y * glyph_pitch;
          for (auto x{ 0 }; x < glyph_width; x++) {
            auto gray_value = row_buffer[x];
            r_buffer[index++] = 0xff;
            r_buffer[index++] = 0xff;
            r_buffer[index++] = 0xff;
            r_buffer[index++] = gray_value;
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
          y_offset += max_row_height + 1;
          _atlas_height += max_row_height + 1;
          max_row_height = 0;
        }

        character.x_offset = x_offset;
        character.y_offset = y_offset;
        x_offset += character.size.x + 1;

        characters[glyph_index] = character;

        offset += _face->glyph->bitmap.width;
      }
      c = FT_Get_Next_Char(_face, c, &glyph_index);
    }

    _atlas_height += max_row_height + 50;

    VkImage image = nullptr;
    _renderer->getAPI()->createFontImage(cmd_buffer, characters, _atlas_width, _atlas_height, image);

    VkImageView imageview = _renderer->getAPI()->createFontImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);

    texture.setName("_plp_font_atlas");
    texture.setImage(image);
    texture.setImageView(imageview);
    texture.setMipLevels(1);
    texture.setWidth(static_cast<uint32_t>(x_offset));
    texture.setHeight(static_cast<uint32_t>(y_offset));
    texture.setIsPublic(true);

    return texture;
  }

  FontCharacter FontManager::get(unsigned int c)
  {
    auto glyph_index = FT_Get_Char_Index(_face, c);

    return characters[glyph_index];
  }

  FontManager::~FontManager()
  {
    FT_Done_Face(_face);
    FT_Done_FreeType(_ft);
  }
}
