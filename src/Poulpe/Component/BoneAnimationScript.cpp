#include "BoneAnimationScript.hpp"

#include "Mesh.hpp"

#include "Poulpe/Core/Log.hpp"

#include <ranges>

namespace Poulpe
{
  static int wrapMove(lua_State* L)
  {
    BoneAnimationScript* animScript = static_cast<BoneAnimationScript*>(lua_touserdata(L, 1));
    auto deltaTime = std::chrono::duration<float, std::milli>(lua_tonumber(L, 2));

    animScript->move(
      animScript->getData(),
      deltaTime);

    return 0;
  }

  BoneAnimationScript::BoneAnimationScript(
    std::vector<Animation> const& animations,
    std::vector<Position> const& positions,
    std::vector<Rotation> const& rotations,
    std::vector<Scale> const& scales)
    : m_Animations(animations)
    , m_Positions(positions)
    , m_Rotations(rotations)
    , m_Scales(scales)
  {
    if (!std::filesystem::exists(m_ScriptPath)) {
      PLP_FATAL("script file {} does not exits.", m_ScriptPath);
      return;
    }

    m_lua_State = luaL_newstate();
    luaL_openlibs(m_lua_State);
    lua_register(m_lua_State, "_Move", wrapMove);

    checkLua(m_lua_State, luaL_dofile(m_lua_State, m_ScriptPath.c_str()));
  }

  BoneAnimationScript::~BoneAnimationScript()
  { 
    lua_close(m_lua_State);
  }

  void BoneAnimationScript::move(
    Data* dataMove,
    std::chrono::duration<float> deltaTimeMove)
  {
    std::unique_ptr<BoneAnimationMove> animMove = std::make_unique<BoneAnimationMove>();

    //PLP_TRACE("START at {}/{}/{}", dataMove->m_OriginPos.x, dataMove->m_OriginPos.y, dataMove->m_OriginPos.z);
    //PLP_TRACE("TO {}/{}/{}", animMove->target.x, animMove->target.y, animMove->target.z);

    animMove->update = [] (
      BoneAnimationMove* anim,
      Data* data,
      std::chrono::duration<float> deltaTime,
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
      anim->elapsedTime += deltaTime.count();
      //PLP_DEBUG("e {} d {} t {} index {} delta {}", anim->elapsedTime, anim->duration, t, index, deltaTime.count());

      //glm::vec3 newPos = glm::mix(data->m_OriginPos, p.value, t);
      //glm::quat newRot = glm::mix(glm::quat(data->m_OriginPos), r.value, t);
      //glm::vec3 newScale = glm::mix(data->m_OriginPos, s.value, t);
      //
      //PLP_DEBUG("Animation: {} {} {}", a.id, a.name, a.duration);
      //PLP_DEBUG("MOVING at {}/{}/{}", newPos.x, newPos.y, newPos.z);
      //PLP_DEBUG("Rotation {}/{}/{}", newRot.x, newRot.y, newRot.z);
      //PLP_DEBUG("Scale {}/{}/{}", newScale.x, newScale.y, newScale.z);

      //data->m_Vertices
      //glm::mat4 model = glm::mat4(1.0f);
      //model = glm::translate(model, newPos);
      //model *= glm::toMat4(newRot);
      //model = glm::scale(model, newScale);

      //std::ranges::for_each(bones, [&data, &index, &positions, &rotations, &scales](auto const& bone) {
      //  std::ranges::for_each(bone.weights, [&data, &index, &bone, &positions, &rotations, &scales](auto const& weight) {
      //    auto& vertex = data->m_Vertices[weight.first];
      //    auto& pos = positions[index];
      //    auto& rot = rotations[index];
      //    auto& scale = scales[index];

      //    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), pos.value);
      //    glm::mat4 rotationMatrix = glm::toMat4(rot.value);
      //    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale.value);
      //    glm::mat4 transform = translationMatrix * rotationMatrix * scaleMatrix * bone.offsetMatrix;

      //    glm::vec4 transformedPosition = transform * glm::vec4(vertex.pos, 1.0f);
      //    vertex.pos += weight.second * glm::vec3(transformedPosition);
      //  });
      //});
      //std::ranges::for_each(data->m_Ubos, [&model](auto& ubo) {
      //  ubo.model = model;
      //});

      //if (done) {
      //  data->m_OriginPos = newPos;
      //  //@todo switch all rotation to glm::quat
      //  data->m_OriginRotation = glm::eulerAngles(newRot);
      //  data->m_OriginScale = newScale;
      //  //PLP_TRACE("DONE at {}/{}/{}", data->m_OriginPos.x, data->m_OriginPos.y, data->m_OriginPos.z);
      //}
      anim->done = done;
    };
    animMove->update(animMove.get(), dataMove, deltaTimeMove, m_Animations, m_Positions, m_Rotations, m_Scales);
    m_NewMoveAnimations.emplace_back(std::move(animMove));
  }

  void BoneAnimationScript::visit(std::chrono::duration<float> deltaTime, IVisitable* mesh)
  {
    m_Data = mesh->getData();

    std::ranges::for_each(m_NewMoveAnimations, [&](auto& anim) {
      m_MoveAnimations.emplace_back(std::move(anim));
    });
 
    m_NewMoveAnimations.clear();

    if (!m_MoveInit) {
      lua_getglobal(m_lua_State, "nextMove");
      if (lua_isfunction(m_lua_State, -1)) {
        lua_pushlightuserdata(m_lua_State, this);
        lua_pushnumber(m_lua_State, static_cast<double>(deltaTime.count()));
        checkLua(m_lua_State, lua_pcall(m_lua_State, 2, 1, 0));
      }
      m_MoveInit = true;
    }

    std::ranges::for_each(m_MoveAnimations, [&](auto& anim) {

      anim->update(anim.get(), mesh->getData(), deltaTime, m_Animations, m_Positions, m_Rotations, m_Scales);

      if (anim->done) {
        lua_getglobal(m_lua_State, "nextMove");
        if (lua_isfunction(m_lua_State, -1)) {
          lua_pushlightuserdata(m_lua_State, this);
          lua_pushnumber(m_lua_State, static_cast<double>(deltaTime.count()));
          checkLua(m_lua_State, lua_pcall(m_lua_State, 2, 1, 0));
        }
      }
    });

    std::erase_if(m_MoveAnimations, [](auto& anim) { return anim->done; });
  }
}
