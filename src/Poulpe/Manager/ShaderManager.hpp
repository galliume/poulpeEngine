#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

#include "vulkan/vulkan.h"

#include <latch>

namespace Poulpe
{
  enum class DescSetLayoutType {
      Skybox, HUD, Entity, Offscreen
  };
  enum class VertexBindingType {
      Vertex2D, Vertex3D
  };

  class ShaderManager
  {
  public:
    ShaderManager();

    inline void addRenderer(Renderer* const renderer)  { _renderer = renderer; }
    void addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath) ;
    void clear();
    inline VulkanShaders* getShaders() const  { return _shaders.get(); }
    inline bool isLoadingDone() { return _LoadingDone.load(); }
    std::function<void(std::latch& count_down)> load(nlohmann::json config) ;

  private:
    template <DescSetLayoutType T>
    VkDescriptorSetLayout createDescriptorSetLayout();
    void createGraphicPipeline(std::string const & shaderName);

    std::vector<VkPipelineShaderStageCreateInfo> getShadersInfo(std::string const & shaderName, bool offscreen = false);

    template <VertexBindingType T>
    VkPipelineVertexInputStateCreateInfo* getVertexInputState();

    nlohmann::json _Config;
    std::atomic_bool _LoadingDone{ false };
    Renderer* _renderer{ nullptr };
    std::unique_ptr<VulkanShaders> _shaders{ nullptr };
  };
}
