#include "AnimationScript.hpp"

#include "Mesh.hpp"

#include "Poulpe/Core/Log.hpp"

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

    animScript->rotate(
      animScript->getData(),
      delta_time,
      duration,
      glm::vec3(angle_x, angle_y, angle_z));

    return 0;
  }

  AnimationScript::AnimationScript(std::string const& scriptPath)
  {
    _script_path = "./" + scriptPath;

    if (!std::filesystem::exists(_script_path)) {
      PLP_FATAL("script file {} does not exits.", _script_path);
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

  void AnimationScript::move(Data* data_move, double delta_time, float duration, glm::vec3 target_move)
  {
    std::unique_ptr<AnimationMove> anim_move = std::make_unique<AnimationMove>();
    anim_move->duration = duration;
    anim_move->target = glm::vec3(
      data_move->_origin_pos.x + target_move.x,
      data_move->_origin_pos.y + target_move.y,
      data_move->_origin_pos.z + target_move.z);

    //PLP_TRACE("START at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
    //PLP_TRACE("TO {}/{}/{}", anim->target.x, anim->target.y, anim->target.z);

    anim_move->update = [](AnimationMove* anim, Data* data, double delta_time) {

      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      
      glm::vec3 target = glm::mix(data->_origin_pos, anim->target, t);
      //PLP_TRACE("MOVING at {}/{}/{}", data->_current_pos.x, data->_current_pos.y, data->_current_pos.z);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data->_origin_scale);
      model = glm::translate(model, target);
      model = glm::rotate(model, glm::radians(data->_origin_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data->_origin_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data->_origin_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      std::ranges::for_each(data->_ubos, [&model](auto& ubo) {
        ubo.model = model;
      });

      anim->elapsedTime += delta_time;

      if (done) {
        data->_origin_pos = target;
        //PLP_TRACE("DONE at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
      }
      anim->done = done;
    };
    anim_move->update(anim_move.get(), data_move, delta_time);
    _new_moves.emplace_back(std::move(anim_move));
  }

  void AnimationScript::rotate(Data* dataRotate, double delta_time, float duration, glm::vec3 angle)
  {
    std::unique_ptr<AnimationRotate> anim_rotate = std::make_unique<AnimationRotate>();
    anim_rotate->duration = duration;
    anim_rotate->angle = angle;

    anim_rotate->update = [](AnimationRotate* anim, Data* data, double delta_time) {

      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += delta_time;

      //@todo switch euler angles to quaternions
      data->_current_rotation = glm::mix(data->_origin_rotation, anim->angle, t);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data->_origin_scale);
      model = glm::translate(model, data->_origin_pos);
      model = glm::rotate(model, glm::radians(data->_current_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data->_current_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data->_current_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      std::ranges::for_each(data->_ubos, [&model](auto& ubo) {
        ubo.model = model;
      });

      anim->done = done;
    };
    anim_rotate->update(anim_rotate.get(), dataRotate, delta_time);
    _new_rotates.emplace_back(std::move(anim_rotate));
  }

  void AnimationScript::operator()(double const delta_time, Mesh* mesh)
  {
    _data = mesh->getData();

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

      anim->update(anim.get(), mesh->getData(), delta_time);

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
      anim->update(anim.get(), mesh->getData(), delta_time);

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
    //lua_State* L = luaL_newstate();
    //luaL_openlibs(L);

    //if (checkLua(L, luaL_dofile(L, _script_path.c_str())))
    //{
    //  lua_getglobal(L, "a");
    //  if (lua_isnumber(L, -1)) {
    //    float result = static_cast<float>(lua_tonumber(L, -1));
    //    PLP_WARN("LUA result: {}", result);
    //  }
    //}

    //lua_getglobal(L, "test");
    //if (lua_isfunction(L, -1)) {
    //  lua_pushnumber(L, 2);
    //  lua_pushnumber(L, 6);

    //  if (checkLua(L, lua_pcall(L, 2, 1, 0))) {
    //    PLP_WARN("test {}", static_cast<int>(lua_tonumber(L, -1)));
    //  }
    //}

    //lua_register(L, "cpptest", lua_cpptest);
    //lua_getglobal(L, "cpptest");
    //if (lua_isfunction(L, -1)) {
    //  lua_pushnumber(L, 10);
    //  lua_pushnumber(L, 20);

    //  if (checkLua(L, lua_pcall(L, 2, 1, 0))) {
    //    PLP_WARN("cpptest {}", static_cast<int>(lua_tonumber(L, -1)));
    //  }
    //}
    //lua_close(L);
}
