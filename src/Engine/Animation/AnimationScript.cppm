export module Engine.Animation.AnimationScript;

import std;

import Engine.Component.Components;

import Engine.Core.AnimationTypes;
import Engine.Core.Constants;
import Engine.Core.Logger;
import Engine.Core.GLM;
import Engine.Core.LUA;
import Engine.Core.MeshTypes;

import Engine.Utils.LuaScript;

namespace Poulpe
{
  export class AnimationScript : public AnimationComponentConcept
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
      glm::vec3 target_angle;
      glm::vec3 start_angle;
      std::function<void(AnimationRotate* anim, Data* data, double delta_time)> update;
    };

    AnimationScript(std::string const & scriptPath);
    ~AnimationScript() override;

    Data* getData();

    void move(Data* data, double delta_time, float duration, glm::vec3 target);
    void rotate(Data* data, double delta_time, float duration, glm::vec3 angle);
    void operator()(AnimationInfo const& animation_info) override;

    void done() override { _done = true; }
    void reset() override { _done = false; }
    void setAnimId(std::uint32_t id) override { _anim_id = id; }

  private:
    Data* _data;
    std::string _script_path;
    lua_State* _lua_State;
    bool _move_init{ false };
    bool _rotate_init{ false };
    bool _done{false};
    std::uint32_t _anim_id{ 0 };
    
    std::vector<std::unique_ptr<AnimationMove>> _moves{};
    std::vector<std::unique_ptr<AnimationMove>> _new_moves{};
    std::vector<std::unique_ptr<AnimationRotate>> _rotates{};
    std::vector<std::unique_ptr<AnimationRotate>> _new_rotates{};
  };
}
