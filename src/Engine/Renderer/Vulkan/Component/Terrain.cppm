module;

#include <vulkan/vulkan.h>
#include <chrono>

export module Engine.Renderer.Vulkan.Terrain;

import Engine.Renderer;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  export class Terrain
  {
  public:
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

      VkShaderStageFlags stage_flag_bits {
        VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT
        | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
        | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT };

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
