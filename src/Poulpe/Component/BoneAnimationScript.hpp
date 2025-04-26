#pragma once
#include "Poulpe/Core/PlpTypedef.hpp"

#include <chrono>

namespace Poulpe
{
  class LightManager;
  class Mesh;
  class Renderer;
  class TextureManager;

  struct BoneAnimation
  {
    bool done{ false };
    float duration{ 0.0f };
    float elapsedTime{ 0.0f };
  };
  struct BoneAnimationMove : public BoneAnimation
  {
    glm::vec3 pos{ 0.0f };
    glm::vec3 rot{ 0.0f };
    glm::vec3 scale{ 0.0f };
    std::function<void(
      Data* data,
      double delta_time,
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<Position>> const& positions,
      std::unordered_map<std::string, std::vector<Rotation>> const& rotations,
      std::unordered_map<std::string, std::vector<Scale>> const& scales)> update;
  };

  class BoneAnimationScript
  {
  public:
    BoneAnimationScript(
      std::vector<Animation> const& animations,
      std::unordered_map<std::string, std::vector<Position>> const& positions,
      std::unordered_map<std::string, std::vector<Rotation>> const& rotations,
      std::unordered_map<std::string, std::vector<Scale>> const& scales);
    ~BoneAnimationScript();

    Data* getData() { return _data; }

    void init(Renderer* const renderer,
       TextureManager* const texture_manager,
       LightManager* const light_manager)
    {
        _renderer = renderer;
    }
    void move(
      Data* dataMove,
      double delta_timeMove);

    void operator()(double const delta_time, Mesh* mesh);

  private:
    Data* _data;
    Renderer* _renderer;

    float _elapsed_time{ 0.f };

    bool _move_init{ false };

    std::vector<std::unique_ptr<BoneAnimationMove>> _moves{};
    std::vector<std::unique_ptr<BoneAnimationMove>> _new_moves{};

    std::vector<Animation> _animations{};
    std::unordered_map<std::string, std::vector<Position>> _positions{};
    std::unordered_map<std::string, std::vector<Rotation>> _rotations{};
    std::unordered_map<std::string, std::vector<Scale>> _scales{};
  };
}
