module;

export module Poulpe.Renderer:Grid;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class Grid : public RendererComponentConcept
  {
  public:
    ~Grid() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;
  
  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
