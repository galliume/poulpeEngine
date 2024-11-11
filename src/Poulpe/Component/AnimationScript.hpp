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

    Data* getData() { return _Data; }

    void init(Renderer* const renderer,
       TextureManager* const texture_manager,
       LightManager* const light_manager)
    {
        _renderer = renderer;
    }
    void move(Data* data, std::chrono::duration<float>, float duration, glm::vec3 target);
    void rotate(Data* data, std::chrono::duration<float>, float duration, glm::vec3 angle);
    void operator()(std::chrono::duration<float> const& deltaTime, Mesh * mesh);

  private:
    Renderer* _renderer;
    std::string _ScriptPath;
    lua_State* _lua_State;
    Data* _Data;
    bool _MoveInit{ false };
    bool _RotateInit{ false };

    std::vector<std::unique_ptr<AnimationMove>> _MoveAnimations{};
    std::vector<std::unique_ptr<AnimationMove>> _NewMoveAnimations{};
    std::vector<std::unique_ptr<AnimationRotate>> _RotateAnimations{};
    std::vector<std::unique_ptr<AnimationRotate>> _NewRotateAnimations{};
  };
}
