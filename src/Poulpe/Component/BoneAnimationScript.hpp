#pragma once

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Utils/LuaScript.hpp"

namespace Poulpe
{
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
        BoneAnimationMove* anim,
        Data* data,
        std::chrono::duration<float> deltaTime,
        std::vector<Animation> const& animations,
        std::vector<Position> const& positions,
        std::vector<Rotation> const& rotations,
        std::vector<Scale> const& scales)> update;
    };

   class BoneAnimationScript : public IVisitor
  {
  public:
    BoneAnimationScript(
      std::vector<Animation> const& animations,
      std::vector<Position> const& positions,
      std::vector<Rotation> const& rotations,
      std::vector<Scale> const& scales);
    ~BoneAnimationScript() override;

    Data* getData() { return m_Data; }

    void init(IRenderer* const renderer,
       ITextureManager* const textureManager,
       ILightManager* const lightManager) override
    {
        m_Renderer = renderer;
    }
    void move(
      Data* dataMove,
      std::chrono::duration<float> deltaTimeMove);

    void visit(std::chrono::duration<float>, IVisitable* mesh) override;

  private:
    std::string const m_ScriptPath{"./assets/scripts/animation/skelet/boneAnimation.lua"};
    Data* m_Data;
    IRenderer* m_Renderer;
    lua_State* m_lua_State;

    bool m_MoveInit{ false };
    
    std::vector<std::unique_ptr<BoneAnimationMove>> m_MoveAnimations{};
    std::vector<std::unique_ptr<BoneAnimationMove>> m_NewMoveAnimations{};

    std::vector<Animation> m_Animations{};
    std::vector<Position> m_Positions{};
    std::vector<Rotation> m_Rotations{};
    std::vector<Scale> m_Scales{};
  };
}
