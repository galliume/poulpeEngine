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
    unsigned int tex_id;
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

    std::unordered_map<char, FontCharacter> characters;

    size_t getAtlasWidth() const { return _atlas_width; }
    size_t getAtlasHeight() const { return _atlas_height; }

    FontManager() = default;
    ~FontManager() ;

    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }

    FontCharacter get(char const c) { return characters[c]; }

    Texture load();

  private:
    Renderer* _renderer{ nullptr };

    size_t _atlas_width{ 0 };
    size_t _atlas_height{ 0 };
  };
}
