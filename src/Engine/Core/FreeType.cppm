module;

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>

export module Engine.Core.FreeType;

#undef FT_LOAD_RENDER
#undef FT_Set_Char_Size

#undef TT_PLATFORM_MICROSOFT
#undef TT_MS_ID_UNICODE_CS

export
{
  inline constexpr int FT_LOAD_RENDER = 0x4;
  inline constexpr int TT_PLATFORM_MICROSOFT = 3;
  inline constexpr int TT_MS_ID_UNICODE_CS = 1;

  using ::FT_RENDER_MODE_SDF;

  using ::FT_Library;
  using ::FT_Face;
  using ::FT_CharMap;
  using ::FT_ULong;
  using ::FT_UInt;
  using ::FT_GlyphSlot;
  using ::FT_Bitmap;
  using ::FT_Vector;
  using ::FT_Error;
  using ::FT_F26Dot6;

  using ::FT_Init_FreeType;
  using ::FT_New_Face;
  using ::FT_Set_Charmap;
  using ::FT_Get_First_Char;
  using ::FT_Get_Next_Char;
  using ::FT_Get_Char_Index;
  using ::FT_Load_Glyph;
  using ::FT_Render_Glyph;
  using ::FT_Load_Char;
  using ::FT_Done_Face;
  using ::FT_Done_FreeType;
  using ::FT_Set_Char_Size;
}
