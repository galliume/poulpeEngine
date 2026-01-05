module;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

module Engine.Animation.BoneAnimationScript;

import std;

namespace Poulpe
{
  BoneAnimationScript::BoneAnimationScript(
    std::unordered_map<std::string, std::vector<Animation>> const& animations,
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::vector<Position>>>> const& positions,
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::vector<Rotation>>>> const& rotations,
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::vector<Scale>>>> const& scales,
    std::uint32_t anim_id
  )
    : _animations(std::move(animations))
    , _positions(std::move(positions))
    , _rotations(std::move(rotations))
    , _scales(std::move(scales))
    , _anim_id(anim_id)
  {

  }

  void BoneAnimationScript::move(
    Data*,
    double)
  {

  }

  void BoneAnimationScript::operator()(AnimationInfo const& animation_info)
  {
    if (_done) return;

    _data = animation_info.data;
    auto const delta_time = animation_info.delta_time;

    std::ranges::for_each(_new_moves, [&](auto& anim) {
      _moves.emplace_back(std::move(anim));
    });

    _new_moves.clear();

    //if (!_move_init) {
    if (_moves.empty() && !_animations[_data->_name].empty()) {
      auto const& anim{ _animations[_data->_name].at(_anim_id) };
      double const duration{ (anim.duration / anim.ticks_per_s) * 1000.0 };//ms

      if (!_first_loop_done) {
        _bone_matrices.resize(_data->_bones.size());
      }

      auto & elapsed_time = _elapsed_time[_data->_name];

      elapsed_time = fmod(elapsed_time + delta_time * 1000.0, duration);
      auto const cache_key { std::trunc(elapsed_time) };

      if (_transform_cache.contains(_data->_name) && _transform_cache[_data->_name].contains(cache_key)) {
        auto const& cache {_transform_cache[_data->_name][cache_key]};
        for (std::size_t i { 0 }; i < _data->_vertices.size(); i++) {
          _data->_vertices[i].pos = cache[i];
        }
      } else {
        //if (_transform_cache[cache_key].empty()) {
          //_transform_cache[cache_key].resize(_data->_vertices.size());
        //}
        auto const& root_bone = _data->_bones[_data->_root_bone_name];
        updateBoneTransforms(anim, root_bone.name, glm::mat4(1.0f), duration, elapsed_time);

        for (auto& vertex : _data->_vertices) {
          float const total_weight {
            vertex.bone_weights[0]
            + vertex.bone_weights[1]
            + vertex.bone_weights[2]
            + vertex.bone_weights[3] };

          if (total_weight > 0.f) {
            glm::vec4 result = glm::vec4(0.0f);
            for (std::size_t i{ 0 }; i < 4; ++i) {
              auto const bone_id{ vertex.bone_ids[i] };
              auto const w { vertex.bone_weights[i] };
              if (w > 0.f) {
                result += _bone_matrices[bone_id] * glm::vec4(vertex.original_pos, 1.0f) * w;
              }
            }
            vertex.pos = glm::vec3(result);
            _transform_cache[_data->_name][cache_key].emplace_back(result);
          } else {
            vertex.pos = vertex.original_pos;
          }
        }
      }


      if (!animation_info.looping && duration < elapsed_time + 10) {
        _done = true;
      }
    }
  }

  void BoneAnimationScript::updateBoneTransforms(
    Animation const& anim,
    std::string const& bone_name,
    glm::mat4 const& parent_transform,
    double const duration,
    double const elapsed_time)
  {
    auto const& bone = _data->_bones[bone_name];

    auto new_scale { bone.t_pose_scale };
    auto new_position { bone.t_pose_position };
    auto new_rotation { bone.t_pose_rotation };

    //Logger::debug("bone: {}", bone.name);
    auto const& scale_bone_node { _scales[_data->_name][bone.name] };
    if (!scale_bone_node.empty() && scale_bone_node.size() >= _anim_id) {
      auto const& scales_data{ scale_bone_node.at(_anim_id) };

      if (!scales_data.empty()) {
        Scale scale_start;
        Scale scale_end;
        std::tie(scale_start, scale_end) = findKeyframe<Scale>(scales_data, elapsed_time);
        new_scale = interpolate<Scale>(scale_start, scale_end, elapsed_time, duration);
      }
    }

    auto const& position_bone_node { _positions[_data->_name][bone.name] };
    if (!position_bone_node.empty() && position_bone_node.size() >= _anim_id) {
      auto const& positions_data = position_bone_node.at(_anim_id);

      if (!positions_data.empty()) {
        Position position_start;
        Position position_end;
        std::tie(position_start, position_end) = findKeyframe<Position>(positions_data, elapsed_time);
        new_position = interpolate<Position>(position_start, position_end, elapsed_time, duration);
      }
    }

    auto const& rotation_bone_node { _rotations[_data->_name][bone.name] };
    if (!rotation_bone_node.empty() && rotation_bone_node.size() >= _anim_id) {
      auto const& rotations_data{ rotation_bone_node.at(_anim_id) };

      if (!rotations_data.empty()) {
        Rotation rotation_start;
        Rotation rotation_end;
        std::tie(rotation_start, rotation_end) = findKeyframe<Rotation>(rotations_data, elapsed_time);
        new_rotation = interpolate<Rotation>(rotation_start, rotation_end, elapsed_time, duration);
      }
    }

    glm::mat4 const S { glm::scale(glm::mat4(1.0f), new_scale) };
    glm::mat4 const R { glm::toMat4(new_rotation) };
    glm::mat4 const T { glm::translate(glm::mat4(1.0f), new_position) };
    glm::mat4 const transform { T * R * S };

    glm::mat4 const global { parent_transform * transform };
    
    if (!bone.is_weightless) {
      _bone_matrices[bone.id] = _data->_inverse_transform_matrix * global * bone.offset_matrix;
    }

    for (auto child : bone.children) {
      updateBoneTransforms(anim, child, global, duration, elapsed_time);
    }
  }

  Data* BoneAnimationScript::getData()
  {
    return _data;
  }
}
