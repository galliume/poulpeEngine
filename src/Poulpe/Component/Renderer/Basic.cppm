module;

#include <vulkan/vulkan.h>
#include <chrono>

class LightManager;
class Mesh;
class Renderer;
class TextureManager;

export module Poulpe.Component.Renderer.Basic;

namespace Poulpe
{
  export class Basic
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
