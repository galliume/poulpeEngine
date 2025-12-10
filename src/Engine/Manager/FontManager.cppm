module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>
#include <glm/glm.hpp>
#include <volk.h>


export module Engine.Managers.FontManager;

import std;

import Engine.Component.Texture;

import Engine.Core.PlpTypedef;

import Engine.Renderer;

namespace Poulpe
{
  export class FontManager
  {
  public:
    struct Text
    {
      std::string name;//must be unique, not robust for the moment
      std::string const text;
      glm::vec3 position { 0.0f };
      glm::vec3 color {1.0f};
      float scale{0.25f};
      bool flat{true}; // flat: 2D UI text, not flat: 3D text
    };

    FontManager() = default;
    ~FontManager();

    std::vector<FontCharacter> const& getCharacters() { return _characters; }
    FT_Face getFace() const& { return _face; }

    uint32_t getAtlasWidth() const { return _atlas_width; }
    uint32_t getAtlasHeight() const { return _atlas_height; }

    inline void addRenderer(Renderer const * renderer) { _renderer = renderer; }

    FontCharacter const& get(uint32_t const c);

    Texture load();

  private:
    std::vector<FontCharacter> _characters;

    Renderer const * _renderer;

    uint32_t _atlas_width{ 0 };
    uint32_t _atlas_height{ 0 };

    FT_Face _face;
    FT_Library _ft;

    //@todo move to config
    FT_F26Dot6 FONT_WIDTH{0};
    FT_F26Dot6 FONT_HEIGHT{ 768 };
    FT_UInt FONT_RESOLUTION{ 512 };
  };
}
