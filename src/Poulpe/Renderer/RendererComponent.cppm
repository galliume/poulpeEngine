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

export module Poulpe.Renderer.RendererComponent;

import Poulpe.Renderer.Mesh;
import Poulpe.Renderer.Renderers;
import Poulpe.Renderer.RendererComponentTypes;
import Poulpe.Renderer.VulkanRenderer;
import Poulpe.Utils.IDHelper;

namespace Poulpe {

  template<typename T>
  concept hasCallOperator = requires(
    T t,
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info) {
    { t(renderer, component_rendering_info) };
  };

  template<typename Class>
  class RenderComponent
  {
  public:
    using ComponentsType = std::variant<
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
    void init(std::unique_ptr<T> impl)
    {
      _id = GUIDGenerator::getGUID();
      _component.emplace<std::unique_ptr<T>>(std::move(impl));
    }

    template<typename T>
    T* has() const {
      if (auto ptr = std::get_if<std::unique_ptr<T>>(&_component)) {
        return ptr->get();
      }
      return nullptr;
    }

    void setOwner(IDType owner) { _owner = owner; }

    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info)
    {
      std::visit([&](auto& component) {
        if constexpr (hasCallOperator<decltype(*component)>) {
          (*component)(renderer, component_rendering_info);
        }
      }, _component);
    }

    VkShaderStageFlags getShaderStageFlags() const
    {
      return std::visit([](auto& ptr) -> VkShaderStageFlags {
        if (ptr) {
            return ptr->stage_flag_bits; 
        }
        return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      }, _component);
    }

  protected:
    ComponentsType _component;

  private:
    IDType _id{ 0 };
    IDType _owner{ 0 };
  };

  export class RendererComponent : public RenderComponent<RendererComponent>
  {

  };
}
