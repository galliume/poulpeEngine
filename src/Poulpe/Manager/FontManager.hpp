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

  struct FontAtlas {
    unsigned int mem_size{ 0 };
    unsigned int width{0};
    unsigned int height{0};
    unsigned int count{0};
    std::array<std::array<unsigned char*, 16>, 16> buffer;

    std::array<unsigned char*, 256> flatten() const
    {
      std::array<unsigned char*, 256> flattened;
      size_t index = 0;

      auto const row_width{ 16 };
      auto const row_height{ 16 };

      for (auto i{ 0 }; i < row_height; ++i) {
        for (auto j{ 0 }; j < row_width; ++j) {
          flattened[i * row_width + j] = buffer[i][j];
        }
      }
      return flattened;
    }
  };

  class FontManager
  {
  public:

    struct character {
      unsigned int tex_id;
      float offset;
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
