module;
#include <volk.h>

export module Poulpe.Renderer:Skybox;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class Skybox : public RendererComponentConcept
  {
  public:
    ~Skybox() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

}
