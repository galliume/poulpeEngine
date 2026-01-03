module;


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

export module Engine.Animation.BoneAnimationScript;

import std;

import Engine.Animation.AnimationTypes;
import Engine.Component.Components;
import Engine.Core.Logger;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;

namespace Poulpe
{
  export struct BoneAnimation
  {
    BoneAnimation() = default;
    ~BoneAnimation() = default;

    bool done{ false };
    double duration{ 0.0 };
    double elapsedTime{ 0.0 };
  };

  export struct BoneAnimationMove : public BoneAnimation
  {
    BoneAnimationMove() = default;
    ~BoneAnimationMove() = default;

    glm::vec3 pos{ 0.0 };
    glm::vec3 rot{ 0.0 };
    glm::vec3 scale{ 0.0 };
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

  export class BoneAnimationScript : public AnimationComponentConcept
  {
  public:
    BoneAnimationScript(
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<std::vector<Position>>> const& positions,
      std::unordered_map<std::string, std::vector<std::vector<Rotation>>> const& rotations,
      std::unordered_map<std::string, std::vector<std::vector<Scale>>> const& scales);
    ~BoneAnimationScript() override = default;

    Data* getData();
    void move(Data* dataMove, double delta_timeMove);

    void operator()(AnimationInfo const& animation_info) override;

  private:
    Data* _data;
    double _elapsed_time{ 0.0 };

    bool _move_init{ false };
    bool _done{false};
    uint32_t _anim_id{ 0 };

    bool _first_loop_done {false};

    std::unordered_map<double, std::vector<glm::vec3>> _transform_cache{};
    
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
      double const duration);

    template<isAnimOperation T>
    std::pair<T, T> findKeyframe(
      std::vector<T> const& key_frames,
      double const time) {

      if (key_frames.size() == 1) {
        return { key_frames[0], key_frames[0] };
      }

      for (std::size_t i{ 0 }; i < key_frames.size() - 1; ++i) {
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
      double current_time,
      double)
    {
      double start_time { start.time };
      double end_time { end.time };

      double duration = end_time - start_time;

      double t = (duration == 0.0) ? 0.0 : (current_time - start_time) / duration;
      t = std::clamp(t, 0.0, 1.0);

      if constexpr (std::same_as<decltype(start.value), glm::vec3>) {
        switch (start.interpolation) {
        case AnimInterpolation::CUBIC_SPLINE:
        case AnimInterpolation::SPHERICAL_LINEAR:
        case AnimInterpolation::STEP:
        case AnimInterpolation::LINEAR:
        default:
          return glm::vec3{
            std::lerp(start.value.x, end.value.x, t),
            std::lerp(start.value.y, end.value.y, t),
            std::lerp(start.value.z, end.value.z, t)
          };
        }
        return start.value;
      } else if constexpr (std::same_as<decltype(start.value), glm::dquat>) {
        switch (start.interpolation) {
        case AnimInterpolation::STEP:
        case AnimInterpolation::LINEAR:
        case AnimInterpolation::CUBIC_SPLINE:
        case AnimInterpolation::SPHERICAL_LINEAR:
        default:
          return glm::normalize(glm::slerp(start.value, end.value, t));
        }
      } else {
        static_assert(false, "Unsupported value type in interpolate()");
      }
      return start.value;
    }
  };

   template auto BoneAnimationScript::interpolate<Rotation>(
    Rotation const& start,
    Rotation const& end,
    double current_time,
    double anim_duration);

  template std::pair<Rotation, Rotation> BoneAnimationScript::findKeyframe<Rotation>(
    std::vector<Rotation> const& key_frames,
    double const time);

  template auto BoneAnimationScript::interpolate<Position>(
    Position const& start,
    Position const& end,
    double current_time,
    double anim_duration);

  template std::pair<Position, Position> BoneAnimationScript::findKeyframe<Position>(
    std::vector<Position> const& key_frames,
    double const time);

  template auto BoneAnimationScript::interpolate<Scale>(
    Scale const& start,
    Scale const& end,
    double current_time,
    double anim_duration);

  template std::pair<Scale, Scale> BoneAnimationScript::findKeyframe<Scale>(
    std::vector<Scale> const& key_frames,
    double const time);
}
