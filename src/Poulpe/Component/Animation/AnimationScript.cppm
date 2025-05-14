module;

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

class LightManager;
class Renderer;
class TextureManager;
struct lua_State;

export module Poulpe.Component.Animation.AnimationScript;

import Poulpe.Component.Mesh;
import Poulpe.Core.PlpTypedef;

namespace Poulpe
{
  export class AnimationScript
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
    ~AnimationScript();

    Data* getData() { return _mesh->getData(); }
    Mesh* getMesh() { return _mesh; }

    void init(Renderer* const renderer,
        TextureManager* const texture_manager,
        LightManager* const light_manager)
    {
        _renderer = renderer;
    }
    void move(Data* data, double delta_time, float duration, glm::vec3 target);
    void rotate(Data* data, double delta_time, float duration, glm::quat angle);
    void operator()(double const delta_time, Mesh * mesh);

  private:
    Renderer* _renderer;
    std::string _script_path;
    lua_State* _lua_State;
    Mesh* _mesh;
    bool _move_init{ false };
    bool _rotate_init{ false };

    std::vector<std::unique_ptr<AnimationMove>> _moves{};
    std::vector<std::unique_ptr<AnimationMove>> _new_moves{};
    std::vector<std::unique_ptr<AnimationRotate>> _rotates{};
    std::vector<std::unique_ptr<AnimationRotate>> _new_rotates{};
  };
}
