module;
#include <volk.h>
#include <vulkan/vulkan.h>

export module Engine.Renderer.Vulkan.Basic;

import std;

import Engine.Component.Texture;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  export class Basic
  {
  public:
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

      VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

    Texture const& getTexture(
      ComponentRenderingInfo const& component_rendering_info,
      std::string const& name) const;
  };
}
