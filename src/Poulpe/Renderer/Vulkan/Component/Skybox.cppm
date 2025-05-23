module;
#include <volk.h>

export module Poulpe.Renderer:Skybox;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Skybox : public RenderComponent
  {
  public:
    ~Skybox();
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Skybox::~Skybox() = default;
}
