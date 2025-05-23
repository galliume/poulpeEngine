module;

export module Poulpe.Renderer:Crosshair;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Crosshair : public RenderComponent
  {
  public:
    ~Crosshair();
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  
  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Crosshair::~Crosshair() = default;
}
