module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

class LightManager;
class Renderer;
class TextureManager;

export module Poulpe.Component.Animation.BoneAnimationScript;

import Poulpe.Component.Mesh;
import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  export struct BoneAnimation
  {
    bool done{ false };
    float duration{ 0.0f };
    float elapsedTime{ 0.0f };
  };

  export struct BoneAnimationMove : public BoneAnimation
  {
    glm::vec3 pos{ 0.0f };
    glm::vec3 rot{ 0.0f };
    glm::vec3 scale{ 0.0f };
    std::function<void(
      Data* data,
      double delta_time,
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<std::vector<Position>>> const& positions,
      std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const& rotations,
      std::unordered_map<std::string, std::vector<std::vector<Scale>>> const& scales)> update;
  };

  template <typename T>
  concept isAnimOperation = std::derived_from<T, AnimOperation>;

  export class BoneAnimationScript
  {
  public:
    BoneAnimationScript(
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<std::vector<Position>>> const& positions,
      std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const& rotations,
      std::unordered_map<std::string, std::vector<std::vector<Scale>>> const& scales);
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

    float _elapsed_time{ 0.0f };

    bool _move_init{ false };
    bool _done{false};
    unsigned int _anim_id{ 0 };

    std::vector<std::unique_ptr<BoneAnimationMove>> _moves{};
    std::vector<std::unique_ptr<BoneAnimationMove>> _new_moves{};

    std::vector<Animation> _animations{};
    std::unordered_map<std::string, std::vector<std::vector<Position>>> _positions{};
    std::unordered_map<std::string, std::vector<std::vector<Rotation>>> _rotations{};
    std::unordered_map<std::string, std::vector<std::vector<Scale>>> _scales{};

    std::vector<glm::mat4> _bone_matrices;


    void updateBoneTransforms(
      Animation const& anim,
      std::string const& bone_name,
      glm::mat4 const& parent_transform,
      float const duration);

    template<isAnimOperation T>
    std::pair<T, T> findKeyframe(
      std::vector<T> const& key_frames,
      float const time,
      float const duration) {

      if (key_frames.size() == 1) {
        return { key_frames[0], key_frames[0] };
      }

      for (auto i{ 0 }; i < key_frames.size() - 1; ++i) {
        if (time >= key_frames[i].time && time <= key_frames[i + 1].time) {
          return { key_frames[i], key_frames[i + 1] };
        }
      }

      return { key_frames.back(), key_frames.front() };
    }

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
        default:
          return glm::normalize(glm::slerp(start.value, end.value, t));

        case AnimInterpolation::CUBIC_SPLINE:
          //@todo
          return start.value;
        }
      } else {
        static_assert(false, "Unsupported value type in interpolate()");
      }
    }
  };
}
