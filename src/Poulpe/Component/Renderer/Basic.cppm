export module Poulpe.Components.Renderer:Basic;

import Poulpe.Core.PlpTypedef;
import Poulpe.Manager.LightManager;
import Poulpe.Manager.TextureManager;
import Poulpe.Renderer.Vulkan.Renderer;

#include <vulkan/vulkan.h>

import <chrono>;

export class Basic
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

private:
  Renderer* _renderer;
  TextureManager* _texture_manager;
  LightManager* _light_manager;
};
