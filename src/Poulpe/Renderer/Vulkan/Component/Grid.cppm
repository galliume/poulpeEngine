module;

export module Poulpe.Renderer:Grid;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Grid : public RenderComponent
  {
  public:
    ~Grid();
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
