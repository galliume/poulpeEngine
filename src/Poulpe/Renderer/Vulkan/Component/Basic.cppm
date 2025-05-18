module;
  #include <volk.h>
// #include <vulkan/vulkan.h>
// #include <chrono>

export module Poulpe.Renderer:Basic;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Basic : public RendererComponentConcept
  {
  public:
    ~Basic() override;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  
    VkShaderStageFlags const stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Basic::~Basic() = default;
}
