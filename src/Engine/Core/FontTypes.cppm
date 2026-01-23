module;

#include <glm/glm.hpp>

export module Engine.Core.FontTypes;

namespace Poulpe
{
  export struct FontCharacter {
    uint32_t index;
    glm::fvec2 size;
    glm::fvec2 bearing;
    long advance;
    uint32_t mem_size{ 0 };
    std::vector<int8_t> buffer;
    int pitch{ 0 };
    int x_offset{ 0 };
    int y_offset{ 0 };
  };
}
