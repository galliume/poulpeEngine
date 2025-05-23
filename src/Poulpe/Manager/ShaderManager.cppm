module;

#include <nlohmann/json.hpp>
#include "vulkan/vulkan.h"

#include <functional>
#include <latch>
#include <memory>
#include <string>
#include <vector>

export module Poulpe.Managers:ShaderManager;

import Poulpe.Core.PlpTypedef;
import Poulpe.Renderer;

namespace Poulpe
{
  export enum class DescSetLayoutType : uint8_t
  {
      Skybox, HUD, Entity,
      Offscreen, Terrain, Water,
      Text
  };
  export enum class VertexBindingType : uint8_t
  {
      Vertex2D, Vertex3D
  };

  export class ShaderManager
  {
  public:
    ShaderManager();

    inline void addRenderer(Renderer* const renderer)  { _renderer = renderer; }
    void addShader(
      std::string const& name,
      std::string const& vert_path,
      std::string const& frag_path,
      std::string const& geom_path,
      std::string const& tese_path,
      std::string const& tesc_path);
    
    void clear();
    inline VulkanShaders* getShaders() const  { return _shaders.get(); }
    std::function<void(std::latch& count_down)> load(nlohmann::json config) ;

  private:
    template <DescSetLayoutType T>
    VkDescriptorSetLayout createDescriptorSetLayout();
    void createGraphicPipeline(std::string const & shaderName);

    std::vector<VkPipelineShaderStageCreateInfo> getShadersInfo(std::string const & shaderName, bool offscreen = false);

    template <VertexBindingType T>
    VkPipelineVertexInputStateCreateInfo* getVertexInputState();

    nlohmann::json _config;
    Renderer* _renderer{ nullptr };
    std::unique_ptr<VulkanShaders> _shaders{ nullptr };
  };
}
