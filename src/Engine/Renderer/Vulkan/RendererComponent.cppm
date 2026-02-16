module;

#include <vulkan/vulkan_core.h>

export module Engine.Renderer.RendererComponent;

import std;

import Engine.Component.Mesh;

import Engine.Core.FreeType;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

import Engine.Renderer.Vulkan.Basic;
import Engine.Renderer.Vulkan.ShadowMap;
import Engine.Renderer.Vulkan.Skybox;
import Engine.Renderer.Vulkan.Terrain;
import Engine.Renderer.Vulkan.Text;
import Engine.Renderer.Vulkan.Water;

import Engine.Utils.IDHelper;

namespace Poulpe {

  template<typename T>
  concept hasCallOperator = requires(
    T t,
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context) {
    { t(renderer, mesh, render_context) };
  };

  template<typename Class>
  class RenderComponent
  {
  public:
    using ComponentsType = std::variant<
      std::unique_ptr<Basic>,
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
      Renderer & renderer,
      Mesh & mesh,
      RendererContext const& render_context)
    {
      std::visit([&](auto& component) {
        if constexpr (hasCallOperator<decltype(*component)>) {
          (*component)(renderer, mesh, render_context);
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

  template class RenderComponent<Basic>;
  template class RenderComponent<Mesh>;
  template class RenderComponent<ShadowMap>;
  template class RenderComponent<Skybox>;
  template class RenderComponent<Terrain>;
  template class RenderComponent<Text>;
  template class RenderComponent<Water>;
  template class RenderComponent<RendererComponent>;
}
