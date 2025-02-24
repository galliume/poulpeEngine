#include "FontManager.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

namespace Poulpe
{
  Texture FontManager::load()
  {
    Texture texture{};

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
      PLP_ERROR("FREETYPE: Could not init FreeType Library");
      return texture;
    }

    FT_Face face;
    if (FT_New_Face(ft, "./assets/fonts/montserrat/Montserrat-Regular.ttf", 0, &face))
    {
      PLP_ERROR("FREETYPE: Failed to load font");
      return texture;
    }

    //FT_Set_Pixel_Sizes(face, 0, 48);
    //FT_Set_Char_Size(face, 0, 16 * 64, 96, 96);


    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
      PLP_ERROR("FREETYTPE: Failed to load Glyph");
      return texture;
    }

    VkCommandPool cmd_pool = _renderer->getAPI()->createCommandPool();
    VkCommandBuffer cmd_buffer = _renderer->getAPI()->allocateCommandBuffers(cmd_pool)[0];
 
    auto offset{0.0f};
    int x_offset{ 0 };
    int y_offset{ 0 };
    int const max_row_width{ 64 };
    int max_row_height{ 0 };
    
    //FT_Set_Pixel_Sizes(face, 10, 10); //@todo find good pixel size

    int a = 0;
    for (FT_ULong c = 0; c < 128; c++) {
      if (a != 0) continue;

      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        PLP_ERROR("FREETYTPE: Failed to load Glyph");
        continue;
      }

      if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
        PLP_ERROR("FREETYTPE: Failed to render Glyph");
        continue;
      }

      if (face->glyph->bitmap.buffer == NULL) {
        PLP_WARN("FREETYPE: empty buffer for char {}", c);
        continue;
      }

      FT_GlyphSlot face_glyph = face->glyph;
      if(FT_Render_Glyph(face_glyph, FT_RENDER_MODE_NORMAL)) {
        PLP_WARN("failed to render glyph");
        continue;
      }

      auto const glyph_width { face_glyph->bitmap.width };
      auto const glyph_height { face_glyph->bitmap.rows };

      std::vector<unsigned char> r_buffer{};
      r_buffer.resize(glyph_width * glyph_height * 4, 0);

      auto buffer = face_glyph->bitmap.buffer;
      auto glyph = buffer;
      int index{ 0 };

      for (auto y{ 0 }; y < glyph_height; y++) {
        buffer = glyph;
        for (auto x{ 0 }; x < glyph_width; x++) {
          auto gray_value = *buffer;
          buffer++;
          r_buffer[index++] = 0xff;
          r_buffer[index++] = 0xff;
          r_buffer[index++] = 0xff;
          r_buffer[index++] = gray_value;
        }
        glyph += face_glyph->bitmap.pitch;
      }

      FontCharacter character {
        c,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        face->glyph->advance.x,
        face->glyph->bitmap.width * face->glyph->bitmap.rows,
        r_buffer,
        0,
        0
      };

      max_row_height = std::max(max_row_height, character.size.y);
     
      if (x_offset + character.size.x >= max_row_width) {
        x_offset = 0;
        y_offset += max_row_height;
        max_row_height = 0;
      }

      character.x_offset = x_offset;
      character.y_offset = y_offset;
      x_offset += character.size.x + 1;

      characters[c] = character;

      offset += face->glyph->bitmap.width + 1;
      a += 1;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    VkImage image = nullptr;
    _renderer->getAPI()->createFontImage(cmd_buffer, characters, image);

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

  FontManager::~FontManager()
  {

  }
}
