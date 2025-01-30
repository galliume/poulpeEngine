#pragma once

#include "Poulpe/Component/AnimationScript.hpp"
#include "Poulpe/Component/BoneAnimationScript.hpp"
#include "Poulpe/Component/Mesh.hpp"

#include "Poulpe/Component/Renderer/Basic.hpp"
#include "Poulpe/Component/Renderer/Crosshair.hpp"
#include "Poulpe/Component/Renderer/Grid.hpp"
#include "Poulpe/Component/Renderer/ShadowMap.hpp"
#include "Poulpe/Component/Renderer/Skybox.hpp"
#include "Poulpe/Component/Renderer/Terrain.hpp"
#include "Poulpe/Component/Renderer/Water.hpp"

#include "Poulpe/Utils/IDHelper.hpp"

#include <variant>

namespace Poulpe
{
  template<typename T>
  concept hasCallOperator = requires(T t, double const delta_time, Mesh * mesh) {
    { t(delta_time, mesh) };
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
      std::unique_ptr<Skybox>,
      std::unique_ptr<Terrain>,
      std::unique_ptr<Water>>;

    IDType getID() const { return _ID; }
    IDType getOwner() const { return _owner; }

    template <typename T>
    void init(std::unique_ptr<T> componentImpl)
    {
      _ID = GUIDGenerator::getGUID();
      _pimpl.emplace<std::unique_ptr<T>>(std::move(componentImpl));
    }

    template<typename T>
    T* has() const {
      if (auto ptr = std::get_if<std::unique_ptr<T>>(&_pimpl)) {
        return ptr->get();
      }
      return nullptr;
    }

    void setOwner(IDType owner) { _owner = owner; }

    void operator()(double const delta_time, Mesh * mesh)
    {
      std::visit([&](auto& component) {
        if constexpr (hasCallOperator<decltype(*component)>) {
          (*component)(delta_time, mesh);
        }
      }, _pimpl);
    }

  protected:
    ComponentImpl _pimpl;

  private:
    IDType _ID{ 0 };
    IDType _owner{ 0 };
  };
}
