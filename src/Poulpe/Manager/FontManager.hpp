#pragma once

#include <Poulpe/Component/Texture.hpp> 

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <volk.h>

#include <unordered_map>

namespace Poulpe
{
  class Renderer;

  struct FontCharacter {
    unsigned int index;
    glm::ivec2 size;
    glm::ivec2 bearing;
    long advance;
    unsigned int mem_size{ 0 };
    std::vector<unsigned char> buffer;
    int pitch{ 0 };
    int x_offset{ 0 };
    int y_offset{ 0 };
  };

  class FontManager
  {
  public:
    struct Text
    {
      std::string name;//must be unique, not robust for the moment
      std::string const text;
      glm::vec3 position { 0.0f };
      glm::vec3 color {1.0f};
      float scale{1.0f};
      bool flat{true}; // flat: 2D UI text, not flat: 3D text
    };

    std::unordered_map<unsigned int, FontCharacter> characters;

    unsigned int getAtlasWidth() const { return _atlas_width; }
    unsigned int getAtlasHeight() const { return _atlas_height; }

    FontManager() = default;
    ~FontManager();

    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }

    FontCharacter get(unsigned int c);

    Texture load();

  private:
    Renderer* _renderer{ nullptr };

    unsigned int _atlas_width{ 0 };
    unsigned int _atlas_height{ 0 };

    FT_Face _face;
    FT_Library _ft;
  };
}
