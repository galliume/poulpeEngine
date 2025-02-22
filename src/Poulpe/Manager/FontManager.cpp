#include "FontManager.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

namespace Poulpe
{
  void FontManager::load()
  {
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
      PLP_ERROR("FREETYPE: Could not init FreeType Library");
      return;
    }

    FT_Face face;
    if (FT_New_Face(ft, "./assets/fonts/montserrat/Montserrat-Regular.ttf", 0, &face))
    {
      PLP_ERROR("FREETYPE: Failed to load font");
      return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
      PLP_ERROR("FREETYTPE: Failed to load Glyph");
      return ;
    }

    VkCommandPool cmd_pool = _renderer->getAPI()->createCommandPool();
    VkCommandBuffer cmd_buffer = _renderer->getAPI()->allocateCommandBuffers(cmd_pool)[0];
 
    for (unsigned char c = 0; c < 128; c++) {

      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        PLP_ERROR("FREETYTPE: Failed to load Glyph");
        continue;
      }

      if (face->glyph->bitmap.buffer == NULL) {
        PLP_WARN("FREETYPE: empty buffer for char {}", c);
        continue;
      }

      VkImage image = nullptr;
      _renderer->getAPI()->createFontImage(cmd_buffer, face, image);

      VkImageView imageview = _renderer->getAPI()->createFontImageView(
        face, image, VK_IMAGE_ASPECT_COLOR_BIT);

      std::string const char_id{ std::to_string(c) };

      Texture texture;
      texture.setName(char_id);
      texture.setImage(image);
      texture.setImageView(imageview);
      texture.setMipLevels(1);
      texture.setWidth(static_cast<uint32_t>(face->glyph->bitmap.width));
      texture.setHeight(static_cast<uint32_t>(face->glyph->bitmap.rows));
      texture.setIsPublic(true);

      character character {
        c,
        texture,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        face->glyph->advance.x
      };
      characters[c] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
  }

  FontManager::~FontManager()
  {

  }
}
