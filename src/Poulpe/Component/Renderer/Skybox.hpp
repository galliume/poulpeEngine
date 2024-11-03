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
      TextureManager* const textureManager,
      LightManager* const lightManager)
    {
      m_Renderer = renderer;
      m_TextureManager = textureManager;
      m_LightManager = lightManager;
    }
      void visit(std::chrono::duration<float> deltaTime, Mesh* mesh);

  private:
    Renderer* m_Renderer;
    TextureManager* m_TextureManager;
    LightManager* m_LightManager;
  };
}
