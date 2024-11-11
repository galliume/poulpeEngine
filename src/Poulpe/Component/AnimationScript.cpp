#include "AnimationScript.hpp"

#include "Mesh.hpp"

#include "Poulpe/Core/Log.hpp"

namespace Poulpe
{
  static int wrapMove(lua_State* L)
  {
    AnimationScript* animScript = static_cast<AnimationScript*>(lua_touserdata(L, 1));
    auto deltaTime = std::chrono::duration<float, std::milli>(lua_tonumber(L, 2));
    float duration = static_cast<float>(lua_tonumber(L, 3));
    float targetX = static_cast<float>(lua_tonumber(L, 4));
    float targetY = static_cast<float>(lua_tonumber(L, 5));
    float targetZ = static_cast<float>(lua_tonumber(L, 6));

    animScript->move(
      animScript->getData(),
      deltaTime,
      duration,
      glm::vec3(targetX, targetY, targetZ));

    return 0;
  }

  static int wrapRotate(lua_State* L)
  {
    AnimationScript* animScript = static_cast<AnimationScript*>(lua_touserdata(L, 1));
    auto deltaTime = std::chrono::duration<float, std::milli>(lua_tonumber(L, 2));
    float duration = static_cast<float>(lua_tonumber(L, 3));
    float angleX = static_cast<float>(lua_tonumber(L, 4));
    float angleY = static_cast<float>(lua_tonumber(L, 5));
    float angleZ = static_cast<float>(lua_tonumber(L, 6));

    animScript->rotate(
      animScript->getData(),
      deltaTime,
      duration,
      glm::vec3(angleX, angleY, angleZ));

    return 0;
  }

  AnimationScript::AnimationScript(std::string const& scriptPath)
  {
    _ScriptPath = "./" + scriptPath;

    if (!std::filesystem::exists(_ScriptPath)) {
      PLP_FATAL("script file {} does not exits.", _ScriptPath);
      return;
    }

    _lua_State = luaL_newstate();
    luaL_openlibs(_lua_State);
    lua_register(_lua_State, "_Rotate", wrapRotate);
    lua_register(_lua_State, "_Move", wrapMove);

    checkLua(_lua_State, luaL_dofile(_lua_State, _ScriptPath.c_str()));
  }

  AnimationScript::~AnimationScript()
  { 
    lua_close(_lua_State);
  }

  void AnimationScript::move(Data* dataMove, std::chrono::duration<float> deltaTimeMove, float duration, glm::vec3 targetMove)
  {
    std::unique_ptr<AnimationMove> animMove = std::make_unique<AnimationMove>();
    animMove->duration = duration;
    animMove->target = glm::vec3(
      dataMove->_origin_pos.x + targetMove.x,
      dataMove->_origin_pos.y + targetMove.y,
      dataMove->_origin_pos.z + targetMove.z);

    //PLP_TRACE("START at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
    //PLP_TRACE("TO {}/{}/{}", anim->target.x, anim->target.y, anim->target.z);

    animMove->update = [](AnimationMove* anim, Data* data, std::chrono::duration<float> deltaTime) {
      
      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += deltaTime.count();

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

      if (done) {
        data->_origin_pos = target;
        //PLP_TRACE("DONE at {}/{}/{}", data->_origin_pos.x, data->_origin_pos.y, data->_origin_pos.z);
      }
      anim->done = done;
    };
    animMove->update(animMove.get(), dataMove, deltaTimeMove);
    _NewMoveAnimations.emplace_back(std::move(animMove));
  }

  void AnimationScript::rotate(Data* dataRotate, std::chrono::duration<float> deltaTimeRotate, float duration, glm::vec3 angle)
  {
    std::unique_ptr<AnimationRotate> animRotate = std::make_unique<AnimationRotate>();
    animRotate->duration = duration;
    animRotate->angle = angle;

    animRotate->update = [](AnimationRotate* anim, Data* data, std::chrono::duration<float> deltaTime) {

      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += deltaTime.count();

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
    animRotate->update(animRotate.get(), dataRotate, deltaTimeRotate);
    _NewRotateAnimations.emplace_back(std::move(animRotate));
  }

  void AnimationScript::operator()(std::chrono::duration<float> const& deltaTime, Mesh* mesh)
  {
    _Data = mesh->getData();

    std::ranges::for_each(_NewMoveAnimations, [&](auto& anim) {
      _MoveAnimations.emplace_back(std::move(anim));
    });
    std::ranges::for_each(_NewRotateAnimations, [&](auto& anim) {
      _RotateAnimations.emplace_back(std::move(anim));
    });

    _NewMoveAnimations.clear();
    _NewRotateAnimations.clear();

    if (!_MoveInit) {
      lua_getglobal(_lua_State, "nextMove");
      if (lua_isfunction(_lua_State, -1)) {
        lua_pushlightuserdata(_lua_State, this);
        lua_pushnumber(_lua_State, static_cast<double>(deltaTime.count()));
        checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
      }
      _MoveInit = true;
    }

    std::ranges::for_each(_MoveAnimations, [&](auto& anim) {

      anim->update(anim.get(), mesh->getData(), deltaTime);

      if (anim->done) {
        lua_getglobal(_lua_State, "nextMove");
        if (lua_isfunction(_lua_State, -1)) {
          lua_pushlightuserdata(_lua_State, this);
          lua_pushnumber(_lua_State, static_cast<double>(deltaTime.count()));
          checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
        }
      }
    });

    if (!_RotateInit) {
      lua_getglobal(_lua_State, "nextRotation");
      if (lua_isfunction(_lua_State, -1)) {
        lua_pushlightuserdata(_lua_State, this);
        lua_pushnumber(_lua_State, static_cast<double>(deltaTime.count()));
        checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
      }
      _RotateInit = true;
    }

    std::ranges::for_each(_RotateAnimations, [&](auto& anim) {
      anim->update(anim.get(), mesh->getData(), deltaTime);

      if (anim->done) {
        lua_getglobal(_lua_State, "nextRotation");
        if (lua_isfunction(_lua_State, -1)) {
          lua_pushlightuserdata(_lua_State, this);
          lua_pushnumber(_lua_State, static_cast<double>(deltaTime.count()));
          checkLua(_lua_State, lua_pcall(_lua_State, 2, 1, 0));
        }
      }
    });

    std::erase_if(_MoveAnimations, [](auto& anim) { return anim->done; });
    std::erase_if(_RotateAnimations, [](auto& anim) { return anim->done; });
  }
    //lua_State* L = luaL_newstate();
    //luaL_openlibs(L);

    //if (checkLua(L, luaL_dofile(L, _ScriptPath.c_str())))
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
