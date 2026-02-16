export module Engine.Renderer.Vulkan.ShadowMap;

import Engine.Component.Mesh;

import Engine.Core.Volk;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  export class ShadowMap
  {
  public:
    void operator()(
      Renderer & renderer,
      Mesh & mesh,
      RendererContext const& render_context);

    VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

  private:
    void createDescriptorSet(
      Renderer & renderer,
      Mesh & mesh,
      RendererContext const& render_context);
  };
}
