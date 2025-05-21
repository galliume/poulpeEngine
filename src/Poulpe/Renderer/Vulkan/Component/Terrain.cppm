module;

#include <vulkan/vulkan.h>
#include <chrono>

export module Poulpe.Renderer:Terrain;

import :VulkanRenderer;

import Poulpe.Component.Components;

namespace Poulpe
{
  export class Terrain : public RendererComponentConcept
  {
  public:
    ~Terrain() override;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
    VkShaderStageFlags const stage_flag_bits { 
      VK_SHADER_STAGE_VERTEX_BIT 
      | VK_SHADER_STAGE_FRAGMENT_BIT
      | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
      | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT};

  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);
  };

  Terrain::~Terrain() = default;
}
