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
      auto const duration{ (anim.duration / anim.ticks_per_s) };//ms

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

      std::ranges::for_each(_data->_bones, [&](auto const& b) {
        auto const& bone = b.second;

        auto new_scale{ glm::vec3(1.0f) };
        auto new_position{ glm::vec3(1.0f) };
        auto new_rotation{ glm::quat() };

        //PLP_DEBUG("bone: {}", bone.name);
        auto const scales_size{ _cache_scales[bone.name].size() };

        if (scales_size >= 1) {
          Scale scale_start{ _cache_scales[bone.name][0] };
          scale_start.interpolation = AnimInterpolation::STEP;

          Scale scale_end{ _cache_scales[bone.name][0] };
          scale_end.time = anim.duration;

          if (scales_size > 1) {
            auto& scale_keys = _cache_scales[bone.name];
            auto it = std::ranges::adjacent_find(scale_keys, [&](auto const& a, auto const& b) {
              return a.time <= _elapsed_time && b.time >= _elapsed_time;
              });

            if (it != scale_keys.end() - 1) {
              scale_start = *it;
              scale_end = *(it + 1);
            }
          }
          new_scale = interpolate<Scale>(scale_start, scale_end, _elapsed_time);
        };

        auto const positions_size{ _cache_positions[bone.name].size() };

        if (positions_size >= 1) {
          Position position_start{ _cache_positions[bone.name][0] };
          position_start.interpolation = AnimInterpolation::STEP;
          Position position_end{ _cache_positions[bone.name][0] };
          position_end.time = anim.duration;

          if (positions_size > 1) {
            auto& position_keys = _cache_positions[bone.name];
            auto it = std::ranges::adjacent_find(position_keys, [&](auto const& a, auto const& b) {
              return a.time <= _elapsed_time && b.time >= _elapsed_time;
              });

            if (it != position_keys.end() - 1) {
              position_start = *it;
              position_end = *(it + 1);
            }
          }
          new_position = interpolate<Position>(position_start, position_end, _elapsed_time);
        }

        auto const rotations_size{ _cache_rotations[bone.name].size() };

        if (rotations_size >= 1) {
          Rotation rotation_start{ _cache_rotations[bone.name][0] };
          rotation_start.interpolation = AnimInterpolation::STEP;

          Rotation rotation_end{ _cache_rotations[bone.name][0] };
          rotation_end.time = anim.duration;

          if (rotations_size > 1) {
            auto& rotation_keys = _cache_rotations[bone.name];
            auto it = std::ranges::adjacent_find(rotation_keys, [&](auto const& a, auto const& b) {
              return a.time <= _elapsed_time && b.time >= _elapsed_time;
              });

            if (it != rotation_keys.end() - 1) {
              rotation_start = *it;
              rotation_end = *(it + 1);
            }
          }

          new_rotation = interpolate<Rotation>(rotation_start, rotation_end, _elapsed_time);
        };

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, new_position);
        transform *= glm::toMat4(new_rotation);
        transform = glm::scale(transform, new_scale);

        glm::mat4 final_transform = _data->_inverse_transform_matrix * transform * bone.offset_matrix;
        _bone_matrices[bone.id] = final_transform;
      });

      for (auto& vertex : _data->_vertices) {
        glm::vec4 result = glm::vec4(0.0f);
        for (auto i{ 0 }; i < 4; ++i) {
          if (i > vertex.bone_ids.size() - 1) break;
          auto bone_id{ vertex.bone_ids[i] };
          auto w{ vertex.bone_weights[i] };
          if (w > 0.0f) {
              result += _bone_matrices[bone_id] * glm::vec4(vertex.pos, 1.0f) * w;
          }
        }
        vertex.pos = glm::vec3(result);
      }

      if (_elapsed_time >= duration) {
        _elapsed_time = 0;
        //t = 1.f;
        //mesh->setIsDirty(false);
        //_done = true;
      } else {
        mesh->setIsDirty();
      }
      _elapsed_time += delta_time;
      //PLP_DEBUG("anim {} elapased time {} duration {} t {} delta {}", anim.name, 
      //_elapsed_time, duration, t, delta_time);
    }
  }
}
