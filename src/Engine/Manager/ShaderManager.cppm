module;
#include "vulkan/vulkan.h"

export module Engine.Managers.ShaderManager;

import std;

import Engine.Core.Json;
import Engine.Core.VulkanTypes;

import Engine.Renderer;

namespace Poulpe
{
  export enum class DescSetLayoutType : std::uint8_t
  {
      Skybox, HUD, Entity,
      Offscreen, Terrain, Water,
      Text, Debug
  };
  export enum class VertexBindingType : std::uint8_t
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
    std::function<void(std::latch& count_down)> load(json config) ;

  private:
    template <DescSetLayoutType T>
    VkDescriptorSetLayout createDescriptorSetLayout();
    void createGraphicPipeline(std::string const & shaderName);

    std::vector<VkPipelineShaderStageCreateInfo> getShadersInfo(std::string const & shaderName, bool offscreen = false);

    template <VertexBindingType T>
    VkPipelineVertexInputStateCreateInfo* getVertexInputState();

    json _config;
    Renderer* _renderer{ nullptr };
    std::unique_ptr<VulkanShaders> _shaders{ nullptr };
  };
}
