module;
#include <volk.h>

export module Poulpe.Renderer:ShadowMap;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class ShadowMap : public RendererComponentConcept
  {
  public:
    ~ShadowMap() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };
  
}
