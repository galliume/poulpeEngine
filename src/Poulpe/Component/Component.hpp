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
      std::unique_ptr<AnimationScript>,
      std::unique_ptr<BoneAnimationScript>,
      std::unique_ptr<Basic>,
      std::unique_ptr<Crosshair>,
      std::unique_ptr<Grid>,
      std::unique_ptr<Mesh>,
      std::unique_ptr<ShadowMap>,
      std::unique_ptr<Skybox>>;

    IDType getID() const { return m_ID; }
    IDType getOwner() const { return m_Owner; }

    template <typename T>
    void init(std::unique_ptr<T> componentImpl)
    {
      m_ID = GUIDGenerator::getGUID();
      m_Pimpl.emplace<std::unique_ptr<T>>(std::move(componentImpl));
    }

    template<typename T>
    T* has() {
      if (auto ptr = std::get_if<T>(m_Pimpl)) {
        return ptr->get();
      }
      return nullptr;
    }

    void setOwner(IDType owner) { m_Owner = owner; }

    template<typename... TArgs>
    void operator()(std::chrono::duration<float> const& deltaTime, TArgs&&... args)
    {
      if constexpr (hasCallOperator<ComponentImpl>) {
        (m_Pimpl)(deltaTime, std::forward<TArgs>(args)...);
      }
    }

  protected:
    ComponentImpl m_Pimpl;

  private:
    IDType m_ID;
    IDType m_Owner;
  };
}
