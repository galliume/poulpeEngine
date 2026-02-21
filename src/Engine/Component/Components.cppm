export module Engine.Component.Components;

import std;

import Engine.Component.Mesh;

import Engine.Core.AnimationTypes;
import Engine.Core.FreeType;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Volk;

import Engine.Utils.IDHelper;

namespace Poulpe {

  export class AnimationComponentConcept
  {
    public:
      AnimationComponentConcept();
      virtual ~AnimationComponentConcept();
      virtual void operator()(AnimationInfo const& animation_info) = 0;
      virtual void done() = 0;
      virtual void reset() = 0;
      virtual void setAnimId(std::uint32_t id) = 0;
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

    Component() = default;

    template <typename T>
    requires isAnimationComponentConcept<T> || std::same_as<T, Mesh>
    Component(std::unique_ptr<T> impl)
    {
      init(std::move(impl));
    }

    Component(Component&&) noexcept = default;
    Component& operator=(Component&&) noexcept = default;

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

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

    void done()
    {
      std::visit([&](auto& component) {
        using T = std::decay_t<decltype(component)>;
        if constexpr (!std::is_same_v<T, std::unique_ptr<Mesh>>) {
          (*component).done();
        }
      }, _component);
    }

    void reset() {
      std::visit([&](auto& component) {
        using T = std::decay_t<decltype(component)>;
        if constexpr (!std::is_same_v<T, std::unique_ptr<Mesh>>) {
        (*component).reset();
        }
      }, _component);
    }

    void setAnimId(std::uint32_t id)
    {
      std::visit([&](auto& component) {
        using T = std::decay_t<decltype(component)>;
        if constexpr (!std::is_same_v<T, std::unique_ptr<Mesh>>) {
          (*component).setAnimId(id);
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

  export template class Component<AnimationComponent>;
  export template class Component<BoneAnimationComponent>;
  export template class Component<MeshComponent>;
}
