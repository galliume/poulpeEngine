module;
#include <memory>
#include <variant>

export module Poulpe.Component.Component;

import Poulpe.Component.Mesh;
import Poulpe.Component.Animation.AnimationScript;
import Poulpe.Component.Animation.BoneAnimationScript;
import Poulpe.Component.Renderer.Basic;
import Poulpe.Component.Renderer.Crosshair;
import Poulpe.Component.Renderer.Grid;
import Poulpe.Component.Renderer.ShadowMap;
import Poulpe.Component.Renderer.Skybox;
import Poulpe.Component.Renderer.Terrain;
import Poulpe.Component.Renderer.Text;
import Poulpe.Component.Renderer.Water;
import Poulpe.Utils.IDHelper;

namespace Poulpe
{
  template<typename T>
  concept hasCallOperator = requires(T t, double const delta_time, Mesh * mesh) {
    { t(delta_time, mesh) };
  };

  export template<typename Class>
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
      std::unique_ptr<Text>,
      std::unique_ptr<Water>>;
    
    IDType getID() const { return _id; }
    IDType getOwner() const { return _owner; }

    template <typename T>
    void init(std::unique_ptr<T> componentImpl)
    {
      _id = GUIDGenerator::getGUID();
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
    IDType _id{ 0 };
    IDType _owner{ 0 };
  };

  export class RenderComponent : public Component<RenderComponent>
  {

  };

  export class MeshComponent : public Component<MeshComponent>
  {

  };

  export class BoneAnimationComponent : public Component<BoneAnimationComponent>
  {

  };

  export class AnimationComponent : public Component<AnimationComponent>
  {
  };
}
