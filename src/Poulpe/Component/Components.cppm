module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>
#include <volk.h>

#include <concepts>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module Poulpe.Component.Components;

import Poulpe.Animation.AnimationTypes;
import Poulpe.Core.PlpTypedef;
import Poulpe.GUI.Window;
import Poulpe.Renderer.Mesh;
import Poulpe.Utils.IDHelper;

namespace Poulpe {

  export class AnimationComponentConcept
  {
    public:
      virtual ~AnimationComponentConcept();
      virtual void operator()(AnimationInfo const& animation_info) = 0;
  };

  AnimationComponentConcept::~AnimationComponentConcept() = default;

  template<typename T>
  concept isAnimationComponentConcept = std::derived_from<T, AnimationComponentConcept>;

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
      _component.emplace<std::unique_ptr<T>>(std::move(impl));
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

    template<typename T>
    void operator()(T&& arg)
    {
      std::visit([&](auto& ptr) {
        (*ptr)(std::forward<T>(arg));
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
}
