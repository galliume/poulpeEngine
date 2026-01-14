module;
#include <glm/glm.hpp>

export module Engine.Managers.LightManager;

import std;

import Engine.Component.Camera;

import Engine.Core.PlpTypedef;

namespace Poulpe
{
  struct CSM
  {
    float d;
    float texel_size;
    float z_max;
    float z_min;
    glm::vec4 s;
    glm::vec4 scale;
    glm::vec4 offset;
    glm::mat4 render;
    glm::mat4 sampling;
  };

  export class LightManager
  {
  public:

    LightManager();

    void animateAmbientLight(double const delta_time);
    void animateSunLight(double const delta_time);

    CSM getLightSpaceMatrix(
    float const near_plane,
    float const far_plane,
    glm::mat4 const & M_camera,
    glm::mat4 const & projection);

    Light getSunLight() { return _sun; }
    std::span<Light> getPointLights() { return _points; }
    std::span<Light> getSpotLights() { return _spots; }

    void computeCSM(glm::mat4 const & camera_view, glm::mat4 const & projection);
    glm::vec3 hsv2rgb(float h, float s, float v);

  private:
    Light _sun{};
    std::vector<Light> _points{};
    std::vector<Light> _spots{};
    float _ambient_hue { 0.0f};
  };
}
