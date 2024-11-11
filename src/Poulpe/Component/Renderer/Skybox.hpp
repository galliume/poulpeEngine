#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Manager/LightManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

#include <vulkan/vulkan.h>

#include <chrono>

namespace Poulpe
{
  class Skybox
  {
    public:
      void createDescriptorSet(Mesh* mesh);
      void setPushConstants(Mesh* mesh);
    void init(
      Renderer* const renderer,
      TextureManager* const texture_manager,
      LightManager* const light_manager)
      {
        _renderer = renderer;
        _texture_manager = texture_manager;
        _light_manager = light_manager;
      }
      void operator()(std::chrono::duration<float> deltaTime, Mesh* mesh);

  private:
    Renderer* _renderer;
    TextureManager* _texture_manager;
    LightManager* _light_manager;
  };
}
