module;

export module Poulpe.Renderer:Grid;

import :VulkanRenderer;

import Poulpe.Component.Components;


namespace Poulpe
{
  export class Grid : public RendererComponentConcept
  {
  public:
    ~Grid() override;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  
  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Grid::~Grid() = default;
}
