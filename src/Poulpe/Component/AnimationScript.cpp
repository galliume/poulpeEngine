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

    //PLP_DEBUG("delta time {} duration {} x {} y {} z {}", delta_time, duration, angle_x, angle_y, angle_z);

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

  static int wrapWave(lua_State* L)
  {
    AnimationScript* animScript = static_cast<AnimationScript*>(lua_touserdata(L, 1));
    double const delta_time{ static_cast<double>(lua_tonumber(L, 2)) };
    float duration = static_cast<float>(lua_tonumber(L, 3));
    float amplitude = static_cast<float>(lua_tonumber(L, 4));
    float lambda = static_cast<float>(lua_tonumber(L, 5));

    PLP_DEBUG("delta time {} duration {} amplitude {} lambda {}", delta_time, duration, amplitude, lambda);

    animScript->wave(
      animScript->getMesh(),
      delta_time,
      duration,
      amplitude,
      lambda);

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
    lua_register(_lua_State, "_Wave", wrapWave);

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

    //PLP_TRACE("START at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
    //PLP_TRACE("TO {}/{}/{}", anim->target.x, anim->target.y, anim->target.z);

    anim->update = [](AnimationMove* anim, Data* data, double delta_time) {

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

      for (auto i{ 0 }; i < data->_ubos.size(); i++) {
        std::ranges::for_each(data->_ubos.at(i), [&model](auto& ubo) {
          ubo.model = model;
        });
      }

      anim->elapsedTime += delta_time;

      if (done) {
        data->_origin_pos = target;
        //PLP_TRACE("DONE at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
      }
      anim->done = done;
    };
    anim->update(anim.get(), data, delta_time);
    _new_moves.emplace_back(std::move(anim));
  }

  void AnimationScript::rotate(Data* data, double delta_time, float duration, glm::quat angle)
  {
    std::unique_ptr<AnimationRotate> anim = std::make_unique<AnimationRotate>();
    anim->duration = duration;
    anim->angle = angle;

    //PLP_DEBUG("START at {}/{}/{}", data->_origin_rotation.x, data->_origin_rotation.y, data->_origin_rotation.z);
    //PLP_DEBUG("TO {}/{}/{}", anim->angle.x, anim->angle.y, anim->angle.z);

    anim->update = [](AnimationRotate* anim, Data* data, double delta_time) {

      float t{ glm::clamp(anim->elapsedTime / anim->duration, 0.0f, 1.0f) };
      bool done{ false };


      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += delta_time;

      data->_current_rotation = glm::mix(data->_origin_rotation, anim->angle, t);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data->_current_scale);
      model = glm::translate(model, data->_current_pos);
      model *= glm::mat4_cast(data->_current_rotation);

      for (auto i{ 0 }; i < data->_ubos.size(); i++) {
        std::ranges::for_each(data->_ubos.at(i), [&model](auto& ubo) {
          ubo.model = model;
        });
      }

      anim->done = done;
    };
    anim->update(anim.get(), data, delta_time);
    _new_rotates.emplace_back(std::move(anim));
  }

  void AnimationScript::wave(Mesh* mesh, double const delta_time, float const duration, float const amplitude, float const lambda)
  {
    std::unique_ptr<AnimationWave> anim = std::make_unique<AnimationWave>();
    anim->duration = duration;
    anim->amplitude = amplitude;
    anim->lambda = lambda;
    anim->mesh = mesh;

    anim->update = [](AnimationWave* anim, double delta_time) {
      float t{ glm::clamp(anim->elapsedTime / anim->duration, 0.0f, 1.0f) };
      
      //PLP_DEBUG("elapsed: {} duration: {} t: {}", anim->elapsedTime, anim->duration, t);
      //PLP_DEBUG("amplitude: {} lambda: {}", anim->amplitude, anim->lambda);
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += delta_time;

      float const speed{ 5.0f * (2.0f / anim->lambda) };
      float const x{ anim->mesh->getData()->_current_pos.y };
      float const omega{ 2.0f / anim->lambda };
      float const phi{ 10.f * (2.0f / anim->lambda)};

      float y = anim->amplitude * sin(x * omega + t * phi);

      y = std::max(y, 0.0f);
      
      anim->mesh->setOptions({x, y, 0.0, 0.0});

      anim->done = done;
      //PLP_DEBUG("y: {}", y);
    };

    anim->update(anim.get(), delta_time);
    _new_waves.emplace_back(std::move(anim));
  }

  void AnimationScript::operator()(double const delta_time, Mesh* mesh)
  {
    _mesh = mesh;
    auto _data = mesh->getData();

    std::ranges::for_each(_new_moves, [&](auto& anim) {
      _moves.emplace_back(std::move(anim));
    });
    std::ranges::for_each(_new_rotates, [&](auto& anim) {
      _rotates.emplace_back(std::move(anim));
    });
    std::ranges::for_each(_new_waves, [&](auto& anim) {
      _waves.emplace_back(std::move(anim));
    });
    _new_moves.clear();
    _new_rotates.clear();
    _new_waves.clear();

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

    if (!_wave_init) {
      lua_getglobal(_lua_State, "nextWave");
      if (lua_isfunction(_lua_State, -1)) {
        lua_pushlightuserdata(_lua_State, this);
        lua_pushnumber(_lua_State, delta_time);
        checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
      }
      _wave_init = true;
    }

    std::ranges::for_each(_waves, [&](auto& anim) {
      anim->update(anim.get(), delta_time);

      if (anim->done) {
        lua_getglobal(_lua_State, "nextWave");
        if (lua_isfunction(_lua_State, -1)) {
          lua_pushlightuserdata(_lua_State, this);
          lua_pushnumber(_lua_State, delta_time);
          checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
        }
      }
    });

    std::erase_if(_moves, [](auto& anim) { return anim->done; });
    std::erase_if(_rotates, [](auto& anim) { return anim->done; });
    std::erase_if(_waves, [](auto& anim) { return anim->done; });
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
