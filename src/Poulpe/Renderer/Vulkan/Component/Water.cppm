module;

#include <vulkan/vulkan.h>
#include <chrono>

export module Poulpe.Renderer:Water;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Water : public RenderComponent
  {
  public:
    ~Water();
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };
  Water::~Water() = default;
}
