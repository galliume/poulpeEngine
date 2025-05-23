module;

#include <vulkan/vulkan.h>
#include <chrono>

export module Poulpe.Renderer:Terrain;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Terrain : public RenderComponent
  {
  public:
    ~Terrain();
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Terrain::~Terrain() = default;
}
