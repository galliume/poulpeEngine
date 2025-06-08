module;

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <concepts>
#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <ranges>
#include <string>
#include <vector>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

module Engine.Animation.AnimationScript;

namespace Poulpe
{
  static int wrapMove(lua_State* L)
  {
    AnimationScript* animScript = static_cast<AnimationScript*>(lua_touserdata(L, 1));
    double const delta_time{ static_cast<double>(lua_tonumber(L, 2)) };
    float duration = static_cast<float>(lua_tonumber(L, 3));
    float targetX = static_cast<float>(lua_tonumber(L, 4));
    float targetY = static_cast<float>(lua_tonumber(L, 5));
    float targetZ = static_cast<float>(lua_tonumber(L, 6));

    animScript->move(
      animScript->getData(),
      delta_time,
      duration,
      glm::vec3(targetX, targetY, targetZ));

    return 0;
  }

  static int wrapRotate(lua_State* L)
  {
    AnimationScript* animScript = static_cast<AnimationScript*>(lua_touserdata(L, 1));
    double const delta_time{ static_cast<double>(lua_tonumber(L, 2)) };
    float duration = static_cast<float>(lua_tonumber(L, 3));
    float angle_x = static_cast<float>(lua_tonumber(L, 4));
    float angle_y = static_cast<float>(lua_tonumber(L, 5));
    float angle_z = static_cast<float>(lua_tonumber(L, 6));

    //Logger::debug("delta time {} duration {} x {} y {} z {}", delta_time, duration, angle_x, angle_y, angle_z);

    glm::quat qx = glm::angleAxis(glm::radians(angle_x), glm::vec3(1, 0, 0));
    glm::quat qy = glm::angleAxis(glm::radians(angle_y), glm::vec3(0, 1, 0));
    glm::quat qz = glm::angleAxis(glm::radians(angle_z), glm::vec3(0, 0, 1));
    
    glm::quat rotation = qx * qy * qz;

    animScript->rotate(
      animScript->getData(),
      delta_time,
      duration,
      rotation);

    return 0;
  }

  AnimationScript::AnimationScript(std::string const& scriptPath)
  {
    _script_path = "./" + scriptPath;

    if (!std::filesystem::exists(_script_path)) {
      Logger::critical("script file {} does not exits.", _script_path);
      return;
    }

    _lua_State = luaL_newstate();
    luaL_openlibs(_lua_State);
    lua_register(_lua_State, "_Rotate", wrapRotate);
    lua_register(_lua_State, "_Move", wrapMove);

    checkLua(_lua_State, luaL_dofile(_lua_State, _script_path.c_str()));
  }

  AnimationScript::~AnimationScript()
  {
    lua_close(_lua_State);
  }

  void AnimationScript::move(Data* data, double delta_time, float duration, glm::vec3 target_move)
  {
    std::unique_ptr<AnimationMove> anim = std::make_unique<AnimationMove>();
    anim->duration = duration;
    anim->target = glm::vec3(
      data->_origin_pos.x + target_move.x,
      data->_origin_pos.y + target_move.y,
      data->_origin_pos.z + target_move.z);

    //Logger::trace("START at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
    //Logger::trace("TO {}/{}/{}", anim->target.x, anim->target.y, anim->target.z);

    anim->update = [](AnimationMove* anim_move, Data* data_move, double dt) {

      float t = anim_move->elapsedTime / anim_move->duration;
      bool done{ false };

      if (anim_move->elapsedTime >= anim_move->duration) {
        done = true;
        t = 1.f;
      }

      glm::vec3 target = glm::mix(data_move->_origin_pos, anim_move->target, t);
      //Logger::trace("MOVING at {}/{}/{}", data->_current_pos.x, data->_current_pos.y, data->_current_pos.z);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data_move->_origin_scale);
      model = glm::translate(model, target);
      model = glm::rotate(model, glm::radians(data_move->_origin_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data_move->_origin_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data_move->_origin_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      for (size_t i{ 0 }; i < data_move->_ubos.size(); i++) {
        std::ranges::for_each(data_move->_ubos.at(i), [&model](auto& ubo) {
          ubo.model = model;
        });
      }

      anim_move->elapsedTime += static_cast<float>(dt);

      if (done) {
        data_move->_origin_pos = target;
        //Logger::trace("DONE at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
      }
      anim_move->done = done;
    };
    anim->update(anim.get(), data, delta_time);
    _new_moves.emplace_back(std::move(anim));
  }

  void AnimationScript::rotate(Data* data, double delta_time, float duration, glm::quat angle)
  {
    std::unique_ptr<AnimationRotate> anim = std::make_unique<AnimationRotate>();
    anim->duration = duration;
    anim->angle = angle;

    //Logger::debug("START at {}/{}/{}", data->_origin_rotation.x, data->_origin_rotation.y, data->_origin_rotation.z);
    //Logger::debug("TO {}/{}/{}", anim->angle.x, anim->angle.y, anim->angle.z);

    anim->update = [](AnimationRotate* anim_rotate, Data* data_rotate, double dt) {

      float t{ glm::clamp(anim_rotate->elapsedTime / anim_rotate->duration, 0.0f, 1.0f) };
      bool done{ false };


      if (anim_rotate->elapsedTime >= anim_rotate->duration) {
        done = true;
        t = 1.f;
      }
      anim_rotate->elapsedTime += static_cast<float>(dt);

      data_rotate->_current_rotation = glm::mix(data_rotate->_origin_rotation, anim_rotate->angle, t);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data_rotate->_current_scale);
      model = glm::translate(model, data_rotate->_current_pos);
      model *= glm::mat4_cast(data_rotate->_current_rotation);

      for (size_t i{ 0 }; i < data_rotate->_ubos.size(); i++) {
        std::ranges::for_each(data_rotate->_ubos.at(i), [&model](auto& ubo) {
          ubo.model = model;
        });
      }

      anim_rotate->done = done;
    };
    anim->update(anim.get(), data, delta_time);
    _new_rotates.emplace_back(std::move(anim));
  }

  void AnimationScript::operator()(AnimationInfo const& animation_info)
  {
    _data = animation_info.data;
    auto const delta_time = animation_info.delta_time;

    std::ranges::for_each(_new_moves, [&](auto& anim) {
      _moves.emplace_back(std::move(anim));
    });
    std::ranges::for_each(_new_rotates, [&](auto& anim) {
      _rotates.emplace_back(std::move(anim));
    });
    _new_moves.clear();
    _new_rotates.clear();

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

      anim->update(anim.get(), _data, delta_time);

      if (anim->done) {
        lua_getglobal(_lua_State, "nextMove");
        if (lua_isfunction(_lua_State, -1)) {
          lua_pushlightuserdata(_lua_State, this);
          lua_pushnumber(_lua_State, delta_time);
          checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
        }
      }
    });

    if (!_rotate_init) {
      lua_getglobal(_lua_State, "nextRotation");
      if (lua_isfunction(_lua_State, -1)) {
        lua_pushlightuserdata(_lua_State, this);
        lua_pushnumber(_lua_State, delta_time);
        checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
      }
      _rotate_init = true;
    }

    std::ranges::for_each(_rotates, [&](auto& anim) {
      anim->update(anim.get(), _data, delta_time);

      if (anim->done) {
        lua_getglobal(_lua_State, "nextRotation");
        if (lua_isfunction(_lua_State, -1)) {
          lua_pushlightuserdata(_lua_State, this);
          lua_pushnumber(_lua_State, delta_time);
          checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
        }
      }
    });

    std::erase_if(_moves, [](auto& anim) { return anim->done; });
    std::erase_if(_rotates, [](auto& anim) { return anim->done; });
  }

  Data* AnimationScript::getData()
  {
    return _data;
  }
  
    //lua_State* L = luaL_newstate();
    //luaL_openlibs(L);

    //if (checkLua(L, luaL_dofile(L, _script_path.c_str())))
    //{
    //  lua_getglobal(L, "a");
    //  if (lua_isnumber(L, -1)) {
    //    float result = static_cast<float>(lua_tonumber(L, -1));
    //    Logger::warn("LUA result: {}", result);
    //  }
    //}

    //lua_getglobal(L, "test");
    //if (lua_isfunction(L, -1)) {
    //  lua_pushnumber(L, 2);
    //  lua_pushnumber(L, 6);

    //  if (checkLua(L, lua_pcall(L, 2, 1, 0))) {
    //    Logger::warn("test {}", static_cast<int>(lua_tonumber(L, -1)));
    //  }
    //}

    //lua_register(L, "cpptest", lua_cpptest);
    //lua_getglobal(L, "cpptest");
    //if (lua_isfunction(L, -1)) {
    //  lua_pushnumber(L, 10);
    //  lua_pushnumber(L, 20);

    //  if (checkLua(L, lua_pcall(L, 2, 1, 0))) {
    //    Logger::warn("cpptest {}", static_cast<int>(lua_tonumber(L, -1)));
    //  }
    //}
    //lua_close(L);
}
