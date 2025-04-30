#include "BoneAnimationScript.hpp"

#include "Mesh.hpp"

#include "Poulpe/Core/Log.hpp"

#include <ranges>

namespace Poulpe
{
  BoneAnimationScript::BoneAnimationScript(
    std::vector<Animation> const& animations,
    std::unordered_map<std::string, std::vector<Position>> const& positions,
    std::unordered_map<std::string, std::vector<Rotation>> const& rotations,
    std::unordered_map<std::string, std::vector<Scale>> const& scales)
    : _animations(std::move(animations))
    , _positions(std::move(positions))
    , _rotations(std::move(rotations))
    , _scales(std::move(scales))
  {

  }

  BoneAnimationScript::~BoneAnimationScript()
  {

  }

  void BoneAnimationScript::move(
    Data* dataMove,
    double delta_timeMove)
  {

  }

  void BoneAnimationScript::operator()(double const delta_time, Mesh* mesh)
  {
    if (_done) return;

    _data = mesh->getData();

    std::ranges::for_each(_new_moves, [&](auto& anim) {
      _moves.emplace_back(std::move(anim));
    });

    _new_moves.clear();

    //if (!_move_init) {
    if (_moves.empty() && !_animations.empty()) {

      auto const& anim{ _animations.at(2) };
      auto const duration{ (anim.duration / anim.ticks_per_s) * 1000.0f };//ms

      if (_cache_scales.empty()) {
        _cache_scales = _scales
          | std::views::filter([&](auto const& pair) {
              auto const& [bone_name, scales_list] = pair;
              return std::ranges::any_of(scales_list, [&](auto const& rot) {
                return rot.animation_ID == anim.id;
                });
            })
          | std::ranges::to<std::unordered_map>();
      }

      if (_cache_positions.empty()) {
        _cache_positions = _positions
          | std::views::filter([&](auto const& pair) {
              auto const& [bone_name, position_list] = pair;
              return std::ranges::any_of(position_list, [&](auto const& rot) {
                return rot.animation_ID == anim.id;
                });
            })
          | std::ranges::to<std::unordered_map>();
        }

      if (_cache_rotations.empty()) {
        _cache_rotations = _rotations
          | std::views::filter([&](auto const& pair) {
              auto const& [bone_name, rotation_list] = pair;
              return std::ranges::any_of(rotation_list, [&](auto const& rot) {
                return rot.animation_ID == anim.id;
                });
            })
          | std::ranges::to<std::unordered_map>();
      }

      _bone_matrices.clear();
      _bone_matrices.resize(_data->_bones.size());
      _elapsed_time = fmod(_elapsed_time, anim.duration);
      updateBoneTransforms(anim, _data->_root_bone_name, glm::mat4(1.0f));

      for (auto& vertex : _data->_vertices) {
        glm::vec4 result = glm::vec4(0.0f);
        for (auto i{ 0 }; i < 4; ++i) {
          auto bone_id{ vertex.bone_ids[i] };
          auto w{ vertex.bone_weights[i] };
          if (w > 0.0f) {
              result += _bone_matrices[bone_id] * glm::vec4(vertex.original_pos, 1.0f) * w;
          }
        }
        vertex.pos = glm::vec3(result);
      }

      if (_elapsed_time >= duration) {
        _elapsed_time = 0.0f;
        //t = 1.f;
        //mesh->setIsDirty(false);.
        //_done = true;
      } else {
        mesh->setIsDirty();
        _elapsed_time += delta_time * 1000.0f;
      }
      //_elapsed_time = std::clamp(_elapsed_time, 0.f, duration);
      //PLP_DEBUG("_elapsed_time {} duration {} ", _elapsed_time, duration);
      //PLP_DEBUG("anim {} elapased time {} duration {} t {} delta {}", anim.name, 
      //_elapsed_time, duration, t, delta_time);
    }
  }

  void BoneAnimationScript::updateBoneTransforms(
    Animation const& anim,
    std::string const& bone_name,
    glm::mat4 const& parent_transform)
  {
    auto const& bone = _data->_bones[bone_name];

    auto new_scale{ glm::vec3(1.0f) };
    auto new_position{ glm::vec3(1.0f) };
    auto new_rotation{ glm::quat(0, 0, 0, 0) };

    //PLP_DEBUG("bone: {}", bone.name);
    auto const& scales_data{ _cache_scales[bone.name] };

    if (!scales_data.empty()) {
      Scale scale_start;
      Scale scale_end;
      std::tie(scale_start, scale_end) = findKeyframe<Scale>(scales_data, fmod(_elapsed_time, anim.duration), anim.duration);
      new_scale = interpolate<Scale>(scale_start, scale_end, _elapsed_time);
    }

    auto const& positions_data = _cache_positions[bone.name];

    if (!positions_data.empty()) {
      Position position_start;
      Position position_end;
      std::tie(position_start, position_end) = findKeyframe<Position>(positions_data, fmod(_elapsed_time, anim.duration), anim.duration);
      new_position = interpolate<Position>(position_start, position_end, _elapsed_time);
    }

    auto const& rotations_data{ _cache_rotations[bone.name] };

    if (!rotations_data.empty()) {
      Rotation rotation_start ;
      Rotation rotation_end;
      std::tie(rotation_start, rotation_end) = findKeyframe<Rotation>(rotations_data, fmod(_elapsed_time, anim.duration), anim.duration);

      if (glm::dot(rotation_start.value, rotation_end.value) < 0.0f) rotation_end.value = -rotation_end.value;
      new_rotation = interpolate<Rotation>(rotation_start, rotation_end, _elapsed_time);
    };

    glm::mat4 const S = glm::scale(glm::mat4(1.0f), new_scale);
    glm::mat4 const R = glm::toMat4(new_rotation);
    glm::mat4 const T = glm::translate(glm::mat4(1.0f), new_position);
    glm::mat4 const transform = T * R * S;

    glm::mat4 global = parent_transform * transform;

    glm::mat4 final_transform = _data->_inverse_transform_matrix * global * bone.offset_matrix;
    _bone_matrices[bone.id] = final_transform;

    for (auto child : bone.children) {
      updateBoneTransforms(anim, child, global);
    }
  }
}
