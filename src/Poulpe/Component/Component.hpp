#pragma once

#include "Poulpe/Component/AnimationScript.hpp"
#include "Poulpe/Component/BoneAnimationScript.hpp"
#include "Poulpe/Component/Mesh.hpp"

#include "Poulpe/Component/Renderer/Basic.hpp"
#include "Poulpe/Component/Renderer/Crosshair.hpp"
#include "Poulpe/Component/Renderer/Grid.hpp"
#include "Poulpe/Component/Renderer/ShadowMap.hpp"
#include "Poulpe/Component/Renderer/Skybox.hpp"

#include "Poulpe/Utils/IDHelper.hpp"

#include <variant>

namespace Poulpe
{
  template<typename T>
  concept hasCallOperator = requires(T t) {
    { t() };
  };

  template<typename Class>
  class Component
  {
  public:
    using ComponentImpl = std::variant<
      AnimationScript,
      BoneAnimationScript,
      Basic,
      Crosshair,
      Grid,
      Mesh,
      ShadowMap,
      Skybox>;

    IDType getID() const { return m_ID; }
    IDType getOwner() const { return m_Owner; }

    void init(ComponentImpl componentImpl)
    {
      m_ID = GUIDGenerator::getGUID();
      m_Pimpl = std::make_unique<decltype(componentImpl)>(componentImpl);
    }

    template<typename T>
    T* has() { return std::get_if<T>(&*m_Pimpl); }

    void setOwner(IDType owner) { m_Owner = owner; }

    template<typename... TArgs>
    void operator()(std::chrono::duration<float> const& deltaTime, TArgs&&... args)
    {
      if constexpr (hasCallOperator<ComponentImpl>) {
        (m_Pimpl)(deltaTime, std::forward<TArgs>(args)...);
      }
    }

  protected:
    std::unique_ptr<ComponentImpl> m_Pimpl;

  private:
    IDType m_ID;
    IDType m_Owner;
  };
}
