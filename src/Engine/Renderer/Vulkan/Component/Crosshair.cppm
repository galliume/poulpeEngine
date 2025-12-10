module;
#include <vulkan/vulkan.h>

export module Engine.Renderer.Vulkan.Crosshair;

import Engine.Renderer;
import Engine.Renderer.RendererComponentTypes;

namespace Poulpe
{
  export class Crosshair
  {
  public:
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

      VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
