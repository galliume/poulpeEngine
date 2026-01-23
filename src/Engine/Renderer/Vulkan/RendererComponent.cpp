module;

#include <vulkan/vulkan_core.h>

module Engine.Renderer.RendererComponent;

import std;

import Engine.Component.Mesh;

import Engine.Renderer.Vulkan.Basic;
import Engine.Renderer.Vulkan.ShadowMap;
import Engine.Renderer.Vulkan.Skybox;
import Engine.Renderer.Vulkan.Terrain;
import Engine.Renderer.Vulkan.Text;
import Engine.Renderer.Vulkan.Water;

namespace Poulpe
{
  template<typename Class>
  RenderComponent<Class>::~RenderComponent() = default;

  RendererComponent::~RendererComponent() = default;

  template<typename Class>
  void RenderComponent<Class>::operator()(
    Renderer *const renderer,
    ComponentRenderingInfo const& component_rendering_info)
  {
    std::visit([&](auto& component) {
      if constexpr (hasCallOperator<decltype(*component)>) {
        (*component)(renderer, component_rendering_info);
      }
    }, _component);
  }

  template<typename Class>
  VkShaderStageFlags RenderComponent<Class>::getShaderStageFlags() const
  {
    return std::visit([](auto& ptr) -> VkShaderStageFlags {
      if (ptr) {
          return ptr->stage_flag_bits;
      }
      return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    }, _component);
  }

  template class RenderComponent<Basic>;
  template class RenderComponent<Mesh>;
  template class RenderComponent<ShadowMap>;
  template class RenderComponent<Skybox>;
  template class RenderComponent<Terrain>;
  template class RenderComponent<Text>;
  template class RenderComponent<Water>;
  template class RenderComponent<RendererComponent>;
}
