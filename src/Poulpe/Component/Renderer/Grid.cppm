module;

#include <vulkan/vulkan.h>
#include <chrono>

class LightManager;
class Renderer;
class TextureManager;

export module Poulpe.Component.Renderer.Grid;

import Poulpe.Component.Mesh;

namespace Poulpe
{
  export class Grid
  {
  public:
    void createDescriptorSet(Mesh* mesh);
    void setPushConstants(Mesh* mesh);
    void init(
      Renderer* renderer,
      TextureManager* texture_manager,
      LightManager* light_manager);
    void operator()(double const delta_time, Mesh* mesh);

  private:
    Renderer* _renderer;
    TextureManager* _texture_manager;
    LightManager* _light_manager;
  };
}
