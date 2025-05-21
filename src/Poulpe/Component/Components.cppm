module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>

#include <concepts>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Renderer;

export module Poulpe.Component.Components;

import Poulpe.Animation.AnimationTypes;
import Poulpe.Component.Camera;
import Poulpe.Component.Texture;
import Poulpe.Core.PlpTypedef;
import Poulpe.GUI.Window;
import Poulpe.Renderer.Mesh;
import Poulpe.Utils.IDHelper;

namespace Poulpe {

  export struct ComponentRenderingInfo
  {
    Mesh* const mesh{nullptr};
    std::unordered_map<std::string, Texture>const& textures;
    std::string const& skybox_name;
    std::string const& terrain_name;
    std::string const& water_name;
    Light const& sun_light;
    std::vector<Light> const& point_lights;
    std::vector<Light> const& spot_lights;
    std::unordered_map<unsigned int, FontCharacter> const& characters;
    FT_Face const& face;
    unsigned int const atlas_width{0};
    unsigned int const atlas_height{0};
  };

  export class RendererComponentConcept
  {
    public:
      virtual ~RendererComponentConcept();
      virtual void operator()(Renderer *const renderer, ComponentRenderingInfo const& rendering_info) = 0;
  };

  RendererComponentConcept::~RendererComponentConcept() = default;

  export class AnimationComponentConcept
  {
    public:
      virtual ~AnimationComponentConcept();
      virtual void operator()(AnimationInfo const& animation_info) = 0;
  };

  AnimationComponentConcept::~AnimationComponentConcept() = default;

  template<typename T>
  concept isRendererComponentConcept = std::derived_from<T, RendererComponentConcept>;

  template<typename T>
  concept isAnimationComponentConcept = std::derived_from<T, AnimationComponentConcept>;

  template<typename Class>
  class Component
  {
  public:
    using ComponentsType = std::variant<
      std::unique_ptr<RendererComponentConcept>,
      std::unique_ptr<AnimationComponentConcept>>;

    IDType getID() const { return _id; }
    IDType getOwner() const { return _owner; }

    template <typename T>
    requires isRendererComponentConcept<T>
      || isAnimationComponentConcept<T>
    void init(std::unique_ptr<T> impl)
    {
      _id = GUIDGenerator::getGUID();
      _component.emplace<std::unique_ptr<T>>(std::move(impl));
    }

    template<typename T>
    requires isRendererComponentConcept<T>
      || isAnimationComponentConcept<T>
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
  export class RenderComponent : public Component<RenderComponent> {};

  export struct RendererInfo
  {
    Mesh* const mesh{nullptr};
    Camera* const camera{nullptr};
    Light const& sun_light;
    std::vector<Light> const& point_lights;
    std::vector<Light> const& spot_lights;
    double elapsed_time{0.0};
    RenderComponent * const render_component;
    bool const normal_debug {false};
  };
}
