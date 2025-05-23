module;

#include <vulkan/vulkan.h>
#include <chrono>

export module Poulpe.Renderer:Terrain;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class Terrain : public RendererComponentConcept
  {
  public:
    ~Terrain() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
