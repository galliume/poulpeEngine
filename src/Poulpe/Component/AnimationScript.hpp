#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Utils/LuaScript.hpp"

#include <chrono>

namespace Poulpe
{
  class LightManager;
  class Mesh;
  class Renderer;
  class TextureManager;

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
      std::function<void(AnimationMove* anim, Data* data, double delta_time)> update;
    };
    struct AnimationRotate : public Animation
    {
      glm::quat angle;
      std::function<void(AnimationRotate* anim, Data* data, double)> update;
    };
    struct AnimationWave : public Animation
    {
      glm::quat angle;
      std::function<void(AnimationWave* anim, Data* data, double)> update;
    };

    AnimationScript(std::string const & scriptPath);
    ~AnimationScript();

    Data* getData() { return _data; }

    void init(Renderer* const renderer,
       TextureManager* const texture_manager,
       LightManager* const light_manager)
    {
        _renderer = renderer;
    }
    void move(Data* data, double delta_time, float duration, glm::vec3 target);
    void rotate(Data* data, double delta_time, float duration, glm::quat angle);
    void wave(Data* data, double delta_time, float duration, glm::quat angle);
    void operator()(double const delta_time, Mesh * mesh);

  private:
    Renderer* _renderer;
    std::string _script_path;
    lua_State* _lua_State;
    Data* _data;
    bool _move_init{ false };
    bool _rotate_init{ false };
    bool _wave_init{ false };

    std::vector<std::unique_ptr<AnimationMove>> _moves{};
    std::vector<std::unique_ptr<AnimationMove>> _new_moves{};
    std::vector<std::unique_ptr<AnimationRotate>> _rotates{};
    std::vector<std::unique_ptr<AnimationRotate>> _new_rotates{};
    std::vector<std::unique_ptr<AnimationWave>> _waves{};
    std::vector<std::unique_ptr<AnimationWave>> _new_waves{};
  };
}
