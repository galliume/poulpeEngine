#pragma once

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Manager/FontManager.hpp"

#include "Poulpe/Renderer/Vulkan/Renderer.hpp"

#include <vulkan/vulkan.h>

#include <chrono>

namespace Poulpe
{
  class Text
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
    void operator()(double const delta_time, Mesh* mesh);

    void addFontManager(FontManager* font_manager) { _font_manager = font_manager; }

  private:
    Renderer* _renderer;
    TextureManager* _texture_manager;
    LightManager* _light_manager;
    FontManager* _font_manager;
  };
}
