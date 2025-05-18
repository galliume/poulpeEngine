module;

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

    std::unordered_map<unsigned int, FontCharacter> getCharacters() { return _characters; }
    FT_Face getFace() { return _face; }
    
    unsigned int getAtlasWidth() const { return _atlas_width; }
    unsigned int getAtlasHeight() const { return _atlas_height; }

    FontManager() = default;
    ~FontManager();

    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }

    FontCharacter get(unsigned int c);

    Texture load();

  private:
    std::unordered_map<unsigned int, FontCharacter> _characters;

    Renderer* _renderer{ nullptr };

    unsigned int _atlas_width{ 0 };
    unsigned int _atlas_height{ 0 };

    FT_Face _face;
    FT_Library _ft;

    //@todo move to config
    unsigned int FONT_HEIGHT{ 768 };
    unsigned int FONT_RESOLUTION{ 512 };
  };
}
