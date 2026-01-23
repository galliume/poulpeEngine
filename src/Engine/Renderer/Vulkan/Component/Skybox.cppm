module;
#include <volk.h>
#include <vulkan/vulkan.h>

export module Engine.Renderer.Vulkan.Skybox;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  export class Skybox
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
