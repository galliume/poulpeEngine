#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include <chrono>

namespace Poulpe
{
  class LightManager;
  class Mesh;
  class Renderer;
  class TextureManager;

  struct BoneAnimation
  {
    bool done{ false };
    float duration{ 0.0f };
    float elapsedTime{ 0.0f };
  };
  struct BoneAnimationMove : public BoneAnimation
  {
    glm::vec3 pos{ 0.0f };
    glm::vec3 rot{ 0.0f };
    glm::vec3 scale{ 0.0f };
    std::function<void(
      Data* data,
      double delta_time,
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<Position>> const& positions,
      std::unordered_map<std::string, std::vector<Rotation>> const& rotations,
      std::unordered_map<std::string, std::vector<Scale>> const& scales)> update;
  };

  template <typename T>
  concept isAnimOperation = std::derived_from<T, AnimOperation>;

  class BoneAnimationScript
  {
  public:
    BoneAnimationScript(
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<Position>> const& positions,
      std::unordered_map<std::string, std::vector<Rotation>> const& rotations,
      std::unordered_map<std::string, std::vector<Scale>> const& scales);
    ~BoneAnimationScript();

    Data* getData() { return _data; }

    void init(Renderer* const renderer,
       TextureManager* const texture_manager,
       LightManager* const light_manager)
    {
      _renderer = renderer;
    }
    void move(
      Data* dataMove,
      double delta_timeMove);

    void operator()(double const delta_time, Mesh* mesh);

  private:
    Data* _data;
    Renderer* _renderer;

    float _elapsed_time{ 0.f };

    bool _move_init{ false };
    bool _done{false};

    std::vector<std::unique_ptr<BoneAnimationMove>> _moves{};
    std::vector<std::unique_ptr<BoneAnimationMove>> _new_moves{};

    std::vector<Animation> _animations{};
    std::unordered_map<std::string, std::vector<Position>> _positions{};
    std::unordered_map<std::string, std::vector<Rotation>> _rotations{};
    std::unordered_map<std::string, std::vector<Scale>> _scales{};

    std::unordered_map<std::string, std::vector<Position>> _cache_positions{};
    std::unordered_map<std::string, std::vector<Rotation>> _cache_rotations{};
    std::unordered_map<std::string, std::vector<Scale>> _cache_scales{};

    std::vector<glm::mat4> _bone_matrices;

    template<isAnimOperation T>
    auto interpolate(
      T const& start,
      T const& end,
      float current_time)
    {
      float duration = end.time - start.time;
      float t = (duration == 0.0f) ? 0.0f : (current_time - start.time) / duration;
      t = std::clamp(t, 0.0f, 1.0f);

      if constexpr (std::same_as<decltype(start.value), glm::vec3>) {
        switch (start.interpolation) {
        case AnimInterpolation::STEP:
          return start.value;

        case AnimInterpolation::LINEAR:
          return glm::vec3{
            std::lerp(start.value.x, end.value.x, t),
            std::lerp(start.value.y, end.value.y, t),
            std::lerp(start.value.z, end.value.z, t)
          };

        case AnimInterpolation::CUBIC_SPLINE:
          //@todo
          return start.value;
        }
        return start.value;
      } else if constexpr (std::same_as<decltype(start.value), glm::quat>) {
        switch (start.interpolation) {
        case AnimInterpolation::STEP:
          return start.value;

        case AnimInterpolation::SPHERICAL_LINEAR:
          return glm::slerp(start.value, end.value, t);

        case AnimInterpolation::CUBIC_SPLINE:
          //@todo
          return start.value;
        default:
          return start.value;
        }
      } else {
        static_assert(false, "Unsupported value type in interpolate()");
      }
    }
  };
}
