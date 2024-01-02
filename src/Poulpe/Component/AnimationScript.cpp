#include "AnimationScript.hpp"

#include "Mesh.hpp"

#include "Poulpe/Core/Log.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Poulpe
{
  static int wrapMove(lua_State* L)
  {
    AnimationScript* animScript = static_cast<AnimationScript*>(lua_touserdata(L, 1));
    float deltaTime = static_cast<float>(lua_tonumber(L, 2));
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
    float deltaTime = static_cast<float>(lua_tonumber(L, 2));
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
    m_ScriptPath = "./" + scriptPath;

    if (!std::filesystem::exists(m_ScriptPath)) {
      PLP_FATAL("script file {} does not exits.", m_ScriptPath);
      return;
    }

    m_lua_State = luaL_newstate();
    luaL_openlibs(m_lua_State);
    lua_register(m_lua_State, "_Rotate", wrapRotate);
    lua_register(m_lua_State, "_Move", wrapMove);

    checkLua(m_lua_State, luaL_dofile(m_lua_State, m_ScriptPath.c_str()));
  }

  AnimationScript::~AnimationScript()
  { 
    lua_close(m_lua_State);
  }

  void AnimationScript::move(Data* data, float deltaTime, float duration, glm::vec3 target)
  {
    std::unique_ptr<AnimationMove> anim = std::make_unique<AnimationMove>();
    anim->duration = duration;
    anim->target = glm::vec3(
      data->m_OriginPos.x + target.x,
      data->m_OriginPos.y + target.y,
      data->m_OriginPos.z + target.z);

    //PLP_TRACE("START at {}/{}/{}", data->m_OriginPos.x, data->m_OriginPos.y, data->m_OriginPos.z);
    //PLP_TRACE("TO {}/{}/{}", anim->target.x, anim->target.y, anim->target.z);

    anim->update = [](AnimationMove* anim, Data* data, float deltaTime) {
      
      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += deltaTime;

      glm::vec3 target = glm::mix(data->m_OriginPos, anim->target, t);
      //PLP_TRACE("MOVING at {}/{}/{}", data->m_CurrentPos.x, data->m_CurrentPos.y, data->m_CurrentPos.z);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data->m_OriginScale);
      model = glm::translate(model, target);
      model = glm::rotate(model, glm::radians(data->m_OriginRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data->m_OriginRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(data->m_OriginRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

      for (auto& ubo : data->m_Ubos) {
        ubo.model = model;
      }

      if (done) {
        data->m_OriginPos = target;
        //PLP_TRACE("DONE at {}/{}/{}", data->m_OriginPos.x, data->m_OriginPos.y, data->m_OriginPos.z);
      }
      anim->done = done;
    };
    anim->update(anim.get(), data, deltaTime);
    m_NewMoveAnimations.emplace_back(std::move(anim));
  }

  void AnimationScript::rotate(Data* data, float deltaTime, float duration, glm::vec3 angle)
  {
    std::unique_ptr<AnimationRotate> anim = std::make_unique<AnimationRotate>();
    anim->duration = duration;
    anim->angle = angle;

    anim->update = [](AnimationRotate* anim, Data* data, float deltaTime) {

      float t = anim->elapsedTime / anim->duration;
      bool done{ false };

      if (anim->elapsedTime >= anim->duration) {
        done = true;
        t = 1.f;
      }
      anim->elapsedTime += deltaTime;

      //@todo switch euler angles to quaternions
      glm::vec3 target = glm::mix(data->m_OriginRotation, anim->angle, t);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, data->m_OriginScale);
      model = glm::translate(model, data->m_OriginPos);
      model = glm::rotate(model, glm::radians(target.x), glm::vec3(1.0f, 0.0f, 0.0f));
      model = glm::rotate(model, glm::radians(target.y), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::rotate(model, glm::radians(target.z), glm::vec3(0.0f, 0.0f, 1.0f));

      for (auto& ubo : data->m_Ubos) {
        ubo.model = model;
      }

      anim->done = done;
    };
    anim->update(anim.get(), data, deltaTime);
    m_NewRotateAnimations.emplace_back(std::move(anim));
  }

  void AnimationScript::visit(float const deltaTime, IVisitable* mesh)
  {
    m_Data = mesh->getData();

    for (auto& anim : m_NewMoveAnimations) {
      m_MoveAnimations.emplace_back(std::move(anim));
    }
    for (auto& anim : m_NewRotateAnimations) {
      m_RotateAnimations.emplace_back(std::move(anim));
    }
    m_NewMoveAnimations.clear();
    m_NewRotateAnimations.clear();

    if (!m_MoveInit) {
      lua_getglobal(m_lua_State, "nextMove");
      if (lua_isfunction(m_lua_State, -1)) {
        lua_pushlightuserdata(m_lua_State, this);
        lua_pushnumber(m_lua_State, deltaTime);
        checkLua(m_lua_State, lua_pcall(m_lua_State, 2, 1, 0));
      }
      m_MoveInit = true;
    }

    for (auto& anim : m_MoveAnimations) {

      anim->update(anim.get(), mesh->getData(), deltaTime);

      if (anim->done) {
        lua_getglobal(m_lua_State, "nextMove");
        if (lua_isfunction(m_lua_State, -1)) {
          lua_pushlightuserdata(m_lua_State, this);
          lua_pushnumber(m_lua_State, deltaTime);
          checkLua(m_lua_State, lua_pcall(m_lua_State, 2, 1, 0));
        }
      }
    }

    if (!m_RotateInit) {
      lua_getglobal(m_lua_State, "nextRotation");
      if (lua_isfunction(m_lua_State, -1)) {
        lua_pushlightuserdata(m_lua_State, this);
        lua_pushnumber(m_lua_State, deltaTime);
        checkLua(m_lua_State, lua_pcall(m_lua_State, 2, 1, 0));
      }
      m_RotateInit = true;
    }

    for (auto& anim : m_RotateAnimations) {

      anim->update(anim.get(), mesh->getData(), deltaTime);

      if (anim->done) {
        lua_getglobal(m_lua_State, "nextRotation");
        if (lua_isfunction(m_lua_State, -1)) {
          lua_pushlightuserdata(m_lua_State, this);
          lua_pushnumber(m_lua_State, deltaTime);
          checkLua(m_lua_State, lua_pcall(m_lua_State, 2, 1, 0));
        }
      }
    }

    std::erase_if(m_MoveAnimations, [](auto& anim) { return anim->done; });
    std::erase_if(m_RotateAnimations, [](auto& anim) { return anim->done; });
  }
    //lua_State* L = luaL_newstate();
    //luaL_openlibs(L);

    //if (checkLua(L, luaL_dofile(L, m_ScriptPath.c_str())))
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