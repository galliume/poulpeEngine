export module Engine.Core.FontTypes;
 
import std;

import Engine.Core.GLM;

namespace Poulpe
{
  export struct FontCharacter {
    std::uint32_t index;
    glm::fvec2 size;
    glm::fvec2 bearing;
    long advance;
    std::uint32_t mem_size{ 0 };
    std::vector<std::int8_t> buffer;
    int pitch{ 0 };
    int x_offset{ 0 };
    int y_offset{ 0 };
  };
}
