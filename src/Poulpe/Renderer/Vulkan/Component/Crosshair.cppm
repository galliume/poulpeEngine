module;

export module Poulpe.Renderer:Crosshair;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class Crosshair : public RendererComponentConcept
  {
  public:
    ~Crosshair() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;
  
  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
