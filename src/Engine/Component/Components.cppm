module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>
#include <volk.h>

export module Engine.Component.Components;

import std;

import Engine.Animation.AnimationTypes;
import Engine.Core.PlpTypedef;
import Engine.GUI.Window;
import Engine.Renderer.Mesh;
import Engine.Utils.IDHelper;

namespace Poulpe {

  export class AnimationComponentConcept
  {
    public:
      AnimationComponentConcept();
      virtual ~AnimationComponentConcept();
      virtual void operator()(AnimationInfo const& animation_info) = 0;
  };

  AnimationComponentConcept::AnimationComponentConcept() = default;
  AnimationComponentConcept::~AnimationComponentConcept() = default;

  template<typename T>
  concept isAnimationComponentConcept = std::derived_from<T, AnimationComponentConcept>;

  template<typename T>
  concept hasCallOperator = requires(
    T t,
    AnimationInfo const& animation_info) {
    { t(animation_info) };
  };

  template<typename Class>
  class Component
  {
  public:
    using ComponentsType = std::variant<
    std::unique_ptr<AnimationComponentConcept>,
    std::unique_ptr<Mesh>>;

    IDType getID() const { return _id; }
    IDType getOwner() const { return _owner; }

    template <typename T>
    requires isAnimationComponentConcept<T>
      || std::same_as<T, Mesh>
    void init(std::unique_ptr<T> impl)
    {
      _id = GUIDGenerator::getGUID();
      if constexpr (isAnimationComponentConcept<T>) {
          _component.emplace<std::unique_ptr<AnimationComponentConcept>>(std::move(impl));
      } else {
          _component.emplace<std::unique_ptr<Mesh>>(std::move(impl));
      }
    }

    template<typename T>
    requires isAnimationComponentConcept<T>
      || std::same_as<T, Mesh>
    T* has() const {
      if (auto ptr = std::get_if<std::unique_ptr<T>>(&_component)) {
        return ptr->get();
      }
      return nullptr;
    }

    void setOwner(IDType owner) { _owner = owner; }

    void operator()(AnimationInfo const& animation_info)
    {
      std::visit([&](auto& component) {
        if constexpr (hasCallOperator<decltype(*component)>) {
          (*component)(animation_info);
        }
      }, _component);
    }

  protected:
    ComponentsType _component;

  private:
    IDType _id{ 0 };
    IDType _owner{ 0 };
  };

  export class AnimationComponent : public Component<AnimationComponent> {};
  export class BoneAnimationComponent : public Component<BoneAnimationComponent> {};
  export class MeshComponent : public Component<MeshComponent> {};

  template class Component<AnimationComponent>;
  template class Component<BoneAnimationComponent>;
  template class Component<MeshComponent>;
}
