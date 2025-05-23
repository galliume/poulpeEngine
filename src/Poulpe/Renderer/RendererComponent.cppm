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

export module Poulpe.Renderer:RendererComponent;

import :VulkanRenderer;

import Poulpe.Component.Texture;
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
    std::unordered_map<uint32_t, FontCharacter> const& characters;
    FT_Face const& face;
    uint32_t const atlas_width{0};
    uint32_t const atlas_height{0};
  };
  
  export class RendererComponentConcept
  {
    public:
      virtual ~RendererComponentConcept();
      virtual void operator()(
        Renderer *const renderer,
        ComponentRenderingInfo const& rendering_info) = 0;

      VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};
  };

  RendererComponentConcept::~RendererComponentConcept() = default;

  template<typename T>
  concept isRendererComponentConcept = std::derived_from<T, RendererComponentConcept>;

  template<typename Class>
  class RendererComponent
  {
  public:
    using ComponentsType = std::variant<
      std::unique_ptr<RendererComponentConcept>>;

    IDType getID() const { return _id; }
    IDType getOwner() const { return _owner; }

    template <typename T>
    requires isRendererComponentConcept<T>
    void init(std::unique_ptr<T> impl)
    {
      _id = GUIDGenerator::getGUID();
      _component.emplace<std::unique_ptr<T>>(std::move(impl));
    }

    template<typename T>
    requires isRendererComponentConcept<T>
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

  export class RenderComponent : public RendererComponent<RenderComponent> 
  {
  };
}
