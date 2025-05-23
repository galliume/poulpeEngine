module;
  #include <volk.h>
// #include <vulkan/vulkan.h>
// #include <chrono>

export module Poulpe.Renderer:Basic;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class Basic : public RendererComponentConcept
  {
  public:
    ~Basic() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
