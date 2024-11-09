#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Utils/LuaScript.hpp"

#include <chrono>

namespace Poulpe
{
  class LightManager;
  class Renderer;
  class TextureManager;
  class Mesh;

  class AnimationScript
  {
  public:
    struct Animation
    {
      bool done{ false };
      float duration{ 0.0f };
      float elapsedTime{ 0.0f };
    };
    struct AnimationMove : public Animation
    {
      glm::vec3 target{ 0.0f };
      std::function<void(AnimationMove* anim, Data* data, std::chrono::duration<float> deltaTime)> update;
    };
    struct AnimationRotate : public Animation
    {
      glm::vec3 angle{ 0.0f };
      std::function<void(AnimationRotate* anim, Data* data, std::chrono::duration<float>)> update;
    };

    AnimationScript(std::string const & scriptPath);
    ~AnimationScript();

    Data* getData() { return m_Data; }

    void init(Renderer* const renderer,
       TextureManager* const textureManager,
       LightManager* const lightManager)
    {
        m_Renderer = renderer;
    }
    void move(Data* data, std::chrono::duration<float>, float duration, glm::vec3 target);
    void rotate(Data* data, std::chrono::duration<float>, float duration, glm::vec3 angle);
    void operator()(std::chrono::duration<float> const& deltaTime, Mesh * mesh);

  private:
    Renderer* m_Renderer;
    std::string m_ScriptPath;
    lua_State* m_lua_State;
    Data* m_Data;
    bool m_MoveInit{ false };
    bool m_RotateInit{ false };

    std::vector<std::unique_ptr<AnimationMove>> m_MoveAnimations{};
    std::vector<std::unique_ptr<AnimationMove>> m_NewMoveAnimations{};
    std::vector<std::unique_ptr<AnimationRotate>> m_RotateAnimations{};
    std::vector<std::unique_ptr<AnimationRotate>> m_NewRotateAnimations{};
  };
}
