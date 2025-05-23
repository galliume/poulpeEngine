module;

#include <cstdint>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>
#include <glm/glm.hpp>
#include <volk.h>
#include <vector>
#include <string>
#include <unordered_map>

export module Poulpe.Managers:FontManager;

import Poulpe.Component.Texture;
import Poulpe.Core.PlpTypedef;
import Poulpe.Renderer;

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

    std::unordered_map<uint32_t, FontCharacter> getCharacters() { return _characters; }
    FT_Face getFace() { return _face; }
    
    uint32_t getAtlasWidth() const { return _atlas_width; }
    uint32_t getAtlasHeight() const { return _atlas_height; }

    FontManager() = default;
    ~FontManager();

    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }

    FontCharacter get(uint32_t c);

    Texture load();

  private:
    std::unordered_map<uint32_t, FontCharacter> _characters;

    Renderer* _renderer{ nullptr };

    uint32_t _atlas_width{ 0 };
    uint32_t _atlas_height{ 0 };

    FT_Face _face;
    FT_Library _ft;

    //@todo move to config
    FT_F26Dot6 FONT_HEIGHT{ 768 };
    FT_F26Dot6 FONT_RESOLUTION{ 512 };
  };
}
