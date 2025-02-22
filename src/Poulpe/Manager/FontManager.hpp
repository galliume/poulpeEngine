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

  class FontManager
  {

  public:

    struct character {
      unsigned int tex_id;
      Texture texture;
      glm::ivec2 size;
      glm::ivec2 bearing;
      long advance;
    };

    std::unordered_map<char, character> characters;

    FontManager() = default;
    ~FontManager() ;

    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }

    character get(char const c) { return characters[c]; }

    void load();

  private:
    Renderer* _renderer{ nullptr };
  };
}
