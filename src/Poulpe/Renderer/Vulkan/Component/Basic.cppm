module;
  #include <volk.h>
// #include <vulkan/vulkan.h>
// #include <chrono>

export module Poulpe.Renderer:Basic;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Basic : public RenderComponent
  {
  public:
    ~Basic();
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Basic::~Basic() = default;
}
