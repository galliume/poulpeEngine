module;
#include <volk.h>
#include <vulkan/vulkan.h>

// #include <chrono>

export module Poulpe.Renderer.Vulkan.Basic;

import Poulpe.Renderer;
import Poulpe.Renderer.RendererComponentTypes;

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
  };
}
