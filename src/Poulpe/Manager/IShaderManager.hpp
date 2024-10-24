#pragma once

#include "Poulpe/Renderer/IRenderer.hpp"

#include <future>
#include <nlohmann/json.hpp>
#include <volk.h>

namespace Poulpe
{
  struct VulkanShaders
  {
    std::unordered_map<std::string, std::vector<VkShaderModule>> shaders;
  };

  class IShaderManager
  {
  public:
    virtual ~IShaderManager() = default;

    virtual inline void addRenderer(IRenderer* const renderer) = 0;
    virtual void addShader(std::string const & name, std::string const & vertPath, std::string const & fragPath) = 0;
    virtual inline VulkanShaders* getShaders() const = 0;
    virtual std::function<void(std::latch& count_down)> load(nlohmann::json config) = 0;
  };
}
