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
    //std::unique_ptr<BoneAnimationMove> animMove = std::make_unique<BoneAnimationMove>();

    //PLP_TRACE("START at {}/{}/{}", dataMove->_origin_pos.x, dataMove->_origin_pos.y, dataMove->_origin_pos.z);

    //animMove->update = [] (
    //  BoneAnimationMove* anim,
    //  Data* data,
    //  double delta_time,
    //  std::vector<Animation> const& animations,
    //  std::vector<Position> const& positions,
    //  std::vector<Rotation> const& rotations,
    //  std::vector<Scale> const& scales) {

    //  if (animations.empty()) {
    //    anim->done = true;
    //    return;
    //  }


    //  //run: from 814.0 to 831.0
    //  anim->duration = 17;//run duration
    //  float t = anim->elapsedTime / anim->duration;
    //  bool done{ false };

    //  unsigned int const index = 814 + static_cast<int>(std::ceil(anim->elapsedTime));

    //  if (anim->elapsedTime >= anim->duration) {
    //    done = true;
    //    t = 1.f;
    //  }
    //  anim->elapsedTime += delta_time;
      //PLP_DEBUG("e {} d {} t {} index {} delta {}", anim->elapsedTime, anim->duration, t, index, delta_time.count());

      //glm::vec3 newPos = glm::mix(data->_origin_pos, p.value, t);
      //glm::quat newRot = glm::mix(glm::quat(data->_origin_pos), r.value, t);
      //glm::vec3 newScale = glm::mix(data->_origin_pos, s.value, t);
      //
      //PLP_DEBUG("Animation: {} {} {}", a.id, a.name, a.duration);
      //PLP_DEBUG("MOVING at {}/{}/{}", newPos.x, newPos.y, newPos.z);
      //PLP_DEBUG("Rotation {}/{}/{}", newRot.x, newRot.y, newRot.z);
      //PLP_DEBUG("Scale {}/{}/{}", newScale.x, newScale.y, newScale.z);

      //data->_vertices
      //glm::mat4 model = glm::mat4(1.0f);
      //model = glm::translate(model, newPos);
      //model *= glm::toMat4(newRot);
      //model = glm::scale(model, newScale);

      //std::ranges::for_each(bones, [&data, &index, &positions, &rotations, &scales](auto const& bone) {
      //  std::ranges::for_each(bone.weights, [&data, &index, &bone, &positions, &rotations, &scales](auto const& weight) {
      //    auto& vertex = data->_vertices[weight.first];
      //    auto& pos = positions[index];
      //    auto& rot = rotations[index];
      //    auto& scale = scales[index];

      //    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), pos.value);
      //    glm::mat4 rotationMatrix = glm::toMat4(rot.value);
      //    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale.value);
      //    glm::mat4 transform = translationMatrix * rotationMatrix * scaleMatrix * bone.offset_matrix;

      //    glm::vec4 transformedPosition = transform * glm::vec4(vertex.pos, 1.0f);
      //    vertex.pos += weight.second * glm::vec3(transformedPosition);
      //  });
      //});
      //std::ranges::for_each(data->_ubos, [&model](auto& ubo) {
      //  ubo.model = model;
      //});

      //if (done) {
      //  data->_origin_pos = newPos;
      //  //@todo switch all rotation to glm::quat
      //  data->_origin_rotation = glm::eulerAngles(newRot);
      //  data->_origin_scale = newScale;
      //  //PLP_TRACE("DONE at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
      //}
     //anim->done = done;
    //};
    //animMove->update(animMove.get(), dataMove, delta_timeMove, _animations, _positions, _rotations, _scales);
    //_new_moves.emplace_back(std::move(animMove));
  }

  void BoneAnimationScript::operator()(double const delta_time, Mesh* mesh)
  {
    _data = mesh->getData();

    std::ranges::for_each(_new_moves, [&](auto& anim) {
      _moves.emplace_back(std::move(anim));
    });

    _new_moves.clear();

    //if (!_move_init) {
    if (_moves.empty() && !_animations.empty()) {

      auto const& anim{ _animations.at(2) };
      auto const duration{ anim.duration * 0.001 };//ms

      float t = _elapsed_time / duration;
      bool done{ false };

      auto positions_by_id = [](
        int anim_id,
        std::unordered_map<std::string, std::vector<Position>> const& positions_map) {
          return positions_map
            | std::views::transform([](auto const& pair) -> auto const& { return pair.second; })
            | std::views::join
            | std::views::filter([=](auto const& pos) { return pos.animation_ID == anim_id; });
        };

      std::ranges::for_each(
        positions_by_id(anim.id, _positions),
        [&](auto const& pos) {
          PLP_DEBUG("anim ID {} time {} x {} y {} z {}", anim.id, pos.time, pos.value.x, pos.value.y, pos.value.z); 

          std::ranges::for_each(_data->_bones, [&](auto const& b) {
            auto const& bone = b.second;
            PLP_DEBUG("bone: {}", bone.name);

            std::ranges::for_each(bone.weights, [&](auto const& weight) {
              auto const& vertex = _data->_vertices.at(weight.vertex_id);
              //vertex.pos = {pos.value.x, pos.value.y, pos.value.z};
            });
          });
        }
      );

      if (_elapsed_time >= duration) {
        _elapsed_time = 0;
        t = 1.f;
      }

      _elapsed_time += delta_time;
      //PLP_DEBUG("anim {} elapased time {} duration {} t {} delta {}", anim.name, 
      //_elapsed_time, duration, t, delta_time);
    }
  }
}
