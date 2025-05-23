module;
#include <volk.h>

export module Poulpe.Renderer:ShadowMap;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class ShadowMap : public RenderComponent
  {
  public:
    ~ShadowMap();
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };
  
  ShadowMap::~ShadowMap() = default;
}
