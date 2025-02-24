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
 
    auto offset{0.0f};

    FontAtlas atlas{};

    atlas.count = face->num_glyphs;

    FT_Set_Pixel_Sizes(face, 10, 10); //@todo find good pixel size

    unsigned int max_width{ 0 };
    unsigned int max_height{ 0 };

    auto const row_width{ 16 };
    auto const row_height{ 16 };

    auto current_width{0};
    auto current_row{0};

    for (FT_ULong c = 0; c < 128; c++) {

      if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        PLP_ERROR("FREETYTPE: Failed to load Glyph");
        continue;
      }

      if (face->glyph->bitmap.buffer == NULL) {
        PLP_WARN("FREETYPE: empty buffer for char {}", c);
        continue;
      }

      character character {
        c,
        offset,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        face->glyph->advance.x
      };
      characters[c] = character;

      offset += face->glyph->bitmap.width + 1;

      atlas.width += offset  ;
      if (atlas.height < face->glyph->bitmap.rows) {
        atlas.height = face->glyph->bitmap.rows;
      }

      auto buffer_size{ face->glyph->bitmap.width * face->glyph->bitmap.rows };
      atlas.mem_size += buffer_size;
      atlas.buffer[current_row][current_width] = face->glyph->bitmap.buffer;

      current_width += 1;

      if (current_width >= row_width) {
        current_width = 0;
        current_row += 1;
      }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    VkImage image = nullptr;
    _renderer->getAPI()->createFontImage(cmd_buffer, atlas, image);

    VkImageView imageview = _renderer->getAPI()->createFontImageView(image, VK_IMAGE_ASPECT_COLOR_BIT);

    Texture texture;
    texture.setName("_plp_font_atlas");
    texture.setImage(image);
    texture.setImageView(imageview);
    texture.setMipLevels(1);
    texture.setWidth(static_cast<uint32_t>(atlas.width));
    texture.setHeight(static_cast<uint32_t>(atlas.height));
    texture.setIsPublic(true);
  }

  FontManager::~FontManager()
  {

  }
}
