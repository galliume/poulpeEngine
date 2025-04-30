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
        _elapsed_time = 0;
        //t = 1.f;
        //mesh->setIsDirty(false);.
        //_done = true;
      } else {
        mesh->setIsDirty();
      }
      _elapsed_time += delta_time * 1000.0f;
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
      auto new_rotation{ glm::quat() };

      //PLP_DEBUG("bone: {}", bone.name);
      auto const& scales_data{ _cache_scales[bone.name] };
      auto const scales_size{ scales_data.size() };

      if (scales_size >= 1) {
        Scale scale_start{ scales_data[0] };
        scale_start.interpolation = AnimInterpolation::STEP;

        Scale scale_end{ scales_data[0] };
        scale_end.time = anim.duration;

        if (scales_size > 1) {
          auto it = std::ranges::adjacent_find(scales_data, [&](auto const& x, auto const& y) {
            return x.time <= _elapsed_time && y.time >= _elapsed_time;
            });

          if (it != scales_data.end()) {
            scale_start = *it;

            if ((it + 1) != scales_data.end()) {
              scale_end = *(it + 1);
            } else {
              scale_end = scales_data.front();
            }
          }
        }
        new_scale = interpolate<Scale>(scale_start, scale_end, _elapsed_time);
      }

      auto const& positions_data = _cache_positions[bone.name];
      auto const positions_size{ positions_data.size() };

      if (positions_size >= 1) {
        Position position_start{ positions_data[0] };
        position_start.interpolation = AnimInterpolation::STEP;
        Position position_end{ positions_data[0] };

        if (positions_size > 1) {
          auto it = std::ranges::adjacent_find(positions_data, [&](auto const& x, auto const& y) {
            return x.time <= _elapsed_time && y.time >= _elapsed_time;
            });

          if (it != positions_data.end()) {
            position_start = *it;

            if ((it + 1) != positions_data.end()) {
              position_end = *(it + 1);
            } else {
              position_end = positions_data.front();
            }
          }
        }
        new_position = interpolate<Position>(position_start, position_end, _elapsed_time);
      }

      auto const& rotations_data{ _cache_rotations[bone.name] };
      auto const rotations_size{ rotations_data.size() };

      if (rotations_size >= 1) {
        Rotation rotation_start{ rotations_data[0] };
        rotation_start.interpolation = AnimInterpolation::STEP;

        Rotation rotation_end{ rotations_data[0] };
        rotation_end.time = anim.duration;

        if (rotations_size > 1) {
          auto it = std::ranges::adjacent_find(rotations_data, [&](auto const& x, auto const& y) {
            return x.time <= _elapsed_time && y.time >= _elapsed_time;
            });

          if (it != rotations_data.end()) {
            rotation_start = *it;

            if ((it + 1) != rotations_data.end()) {
              rotation_end = *(it + 1);
            } else {
              rotation_end = rotations_data.front();
            }
          }
        }
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
