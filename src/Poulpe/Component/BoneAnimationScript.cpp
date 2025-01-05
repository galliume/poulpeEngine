#include "BoneAnimationScript.hpp"

#include "Mesh.hpp"

#include "Poulpe/Core/Log.hpp"

#include <ranges>

namespace Poulpe
{
  static int wrapMove(lua_State* L)
  {
    BoneAnimationScript* animScript = static_cast<BoneAnimationScript*>(lua_touserdata(L, 1));
    double const delta_time{ static_cast<double>(lua_tonumber(L, 2)) };

    animScript->move(
      animScript->getData(),
      delta_time);

    return 0;
  }

  BoneAnimationScript::BoneAnimationScript(
    std::vector<Animation> const& animations,
    std::vector<Position> const& positions,
    std::vector<Rotation> const& rotations,
    std::vector<Scale> const& scales)
    : _Animations(animations)
    , _Positions(positions)
    , _Rotations(rotations)
    , _Scales(scales)
  {
    if (!std::filesystem::exists(_script_path)) {
      PLP_FATAL("script file {} does not exits.", _script_path);
      return;
    }

    _lua_State = luaL_newstate();
    luaL_openlibs(_lua_State);
    lua_register(_lua_State, "_Move", wrapMove);

    checkLua(_lua_State, luaL_dofile(_lua_State, _script_path.c_str()));
  }

  BoneAnimationScript::~BoneAnimationScript()
  {
    lua_close(_lua_State);
  }

  void BoneAnimationScript::move(
    Data* dataMove,
    double delta_timeMove)
  {
    std::unique_ptr<BoneAnimationMove> animMove = std::make_unique<BoneAnimationMove>();

    //PLP_TRACE("START at {}/{}/{}", dataMove->_origin_pos.x, dataMove->_origin_pos.y, dataMove->_origin_pos.z);
    //PLP_TRACE("TO {}/{}/{}", animMove->target.x, animMove->target.y, animMove->target.z);

    animMove->update = [] (
      BoneAnimationMove* anim,
      Data* data,
      double delta_time,
      std::vector<Animation> const& animations,
      std::vector<Position> const& positions,
      std::vector<Rotation> const& rotations,
      std::vector<Scale> const& scales) {

      if (animations.empty()) {
        anim->done = true;
        return;
      }


      //run: from 814.0 to 831.0
      anim->duration = 17;//run duration
      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      unsigned int const index = 814 + static_cast<int>(std::ceil(anim->elapsedTime));

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += delta_time;
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
      anim->done = done;
    };
    animMove->update(animMove.get(), dataMove, delta_timeMove, _Animations, _Positions, _Rotations, _Scales);
    _new_moves.emplace_back(std::move(animMove));
  }

  void BoneAnimationScript::operator()(double const delta_time, Mesh* mesh)
  {
    _data = mesh->getData();

    std::ranges::for_each(_new_moves, [&](auto& anim) {
      _moves.emplace_back(std::move(anim));
    });

    _new_moves.clear();

    if (!_move_init) {
      lua_getglobal(_lua_State, "nextMove");
      if (lua_isfunction(_lua_State, -1)) {
        lua_pushlightuserdata(_lua_State, this);
        lua_pushnumber(_lua_State, delta_time);
        checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
      }
      _move_init = true;
    }

    std::ranges::for_each(_moves, [&](auto& anim) {

      anim->update(anim.get(), mesh->getData(), delta_time, _Animations, _Positions, _Rotations, _Scales);

      if (anim->done) {
        lua_getglobal(_lua_State, "nextMove");
        if (lua_isfunction(_lua_State, -1)) {
          lua_pushlightuserdata(_lua_State, this);
          lua_pushnumber(_lua_State, delta_time);
          checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
        }
      }
    });

    std::erase_if(_moves, [](auto& anim) { return anim->done; });
  }
}
