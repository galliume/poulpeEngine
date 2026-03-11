export module Engine.Core.LightTypes;

import std;

import Engine.Core.GLM;

namespace Poulpe
{
  export struct alignas(16) Light
  {
    glm::mat4 light_space_matrix; // front
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 light_space_matrix_left;
    glm::mat4 light_space_matrix_top;
    glm::mat4 light_space_matrix_right;
    glm::mat4 light_space_matrix_bottom;
    glm::mat4 light_space_matrix_back;
    glm::mat4 cascade0;

    glm::vec4 ads;              // xyz = ads, w = padding
    glm::vec4 clq;              // xyz = clq, w = padding
    glm::vec4 coB{ 1.0f, 0.0f, 0.0f, 0.0f }; // xyz = cutOff/outer/blank, w = padding
    glm::vec4 color;            // xyz = rgb, w = intensity/brightness
    glm::vec4 direction;        // xyz = dir, w = padding
    glm::vec4 position;         // xyz = pos, w = light_type (e.g. 0=Sun, 1=Point)

    glm::vec4 cascade_scale1;   // xyz = scale, w = padding
    glm::vec4 cascade_scale2;
    glm::vec4 cascade_scale3;
    glm::vec4 cascade_offset1;  // xyz = offset, w = padding
    glm::vec4 cascade_offset2;
    glm::vec4 cascade_offset3;

    glm::vec4 cascade_min_splits; // Already vec4 - perfect
    glm::vec4 cascade_max_splits;
    glm::vec4 cascade_texel_sizes;
  };

  export struct alignas(16) LightObjectBuffer
  {
    std::array<Light, 3> lights;
  };

  export enum class SHADOW_TYPE {
    SPOT_LIGHT,
    POINT_LIGHT,
    CSM
  };
}
