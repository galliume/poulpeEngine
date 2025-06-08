module;

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

export module Engine.Animation.AnimationScript;

import Engine.Animation.AnimationTypes;
import Engine.Component.Components;
import Engine.Core.Logger;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
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
      glm::quat angle;
      std::function<void(AnimationRotate* anim, Data* data, double delta_time)> update;
    };

    AnimationScript(std::string const & scriptPath);
    ~AnimationScript() override;

    Data* getData();

    void move(Data* data, double delta_time, float duration, glm::vec3 target);
    void rotate(Data* data, double delta_time, float duration, glm::quat angle);
    void operator()(AnimationInfo const& animation_info) override;

  private:
    Data* _data;
    std::string _script_path;
    lua_State* _lua_State;
    bool _move_init{ false };
    bool _rotate_init{ false };

    std::vector<std::unique_ptr<AnimationMove>> _moves{};
    std::vector<std::unique_ptr<AnimationMove>> _new_moves{};
    std::vector<std::unique_ptr<AnimationRotate>> _rotates{};
    std::vector<std::unique_ptr<AnimationRotate>> _new_rotates{};
  };
}
