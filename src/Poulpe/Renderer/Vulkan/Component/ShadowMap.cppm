module;
#include <volk.h>

export module Poulpe.Renderer:ShadowMap;

import :VulkanRenderer;

import Poulpe.Component.Components;


namespace Poulpe
{
  export class ShadowMap : public RendererComponentConcept
  {
  public:
    ~ShadowMap() override;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
    VkShaderStageFlags const stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };
  
  ShadowMap::~ShadowMap() = default;
}
