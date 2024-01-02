#pragma once

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Utils/LuaScript.hpp"

namespace Poulpe
{
   class AnimationScript : public IVisitor
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

    void init(IRenderer* const renderer,
      [[maybe_unused]] ITextureManager* const textureManager,
      [[maybe_unused]] ILightManager* const lightManager) override
    {
        m_Renderer = renderer;
    }
    void move(Data* data, std::chrono::duration<float>, float duration, glm::vec3 target);
    void rotate(Data* data, std::chrono::duration<float>, float duration, glm::vec3 angle);
    void visit(std::chrono::duration<float>, IVisitable* mesh) override;

  private:
    IRenderer* m_Renderer;
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