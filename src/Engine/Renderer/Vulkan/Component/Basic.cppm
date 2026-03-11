export module Engine.Renderer.Vulkan.Basic;

import std;

import Engine.Component.Mesh;
import Engine.Component.Texture;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;
import Engine.Core.Volk;

namespace Poulpe
{
  export class Basic
  {
  public:
    void operator()(
      Renderer & renderer,
      Mesh & mesh,
      RendererContext const& render_context);

      VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };

  private:
    void createDescriptorSet(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context);

    Texture const& getTexture(
      RendererContext const& render_context,
      std::string const& name) const;
  };
}
