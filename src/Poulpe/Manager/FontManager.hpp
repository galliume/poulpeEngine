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
    int x_offset{ 0 };
    int y_offset{ 0 };
  };

  class FontManager
  {
  public:

    std::unordered_map<char, FontCharacter> characters;

    FontManager() = default;
    ~FontManager() ;

    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }

    FontCharacter get(char const c) { return characters[c]; }

    Texture load();

  private:
    Renderer* _renderer{ nullptr };
  };
}
