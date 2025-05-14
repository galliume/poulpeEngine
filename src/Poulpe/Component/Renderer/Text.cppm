module;

#include <chrono>
#include <glm/glm.hpp>
#include <string_view>
#include <volk.h>

class FontManager;
class LightManager;
class TextureManager;
class Renderer;

export module Poulpe.Component.Renderer.Text;

import Poulpe.Component.Mesh;
//import Poulpe.Renderer.Vulkan.Renderer;

namespace Poulpe
{
  export class Text
  {
  public:
    void createDescriptorSet(Mesh* mesh);
    void setPushConstants(Mesh* mesh);
    void init(
      Renderer* renderer,
      TextureManager* texture_manager,
      LightManager* light_manager);
    void operator()(double const delta_time, Mesh* mesh);

    void addFontManager(FontManager* font_manager);

    void setText(std::string_view text) { _text = text; }
    void setPosition(glm::vec3 const& position) { _position = position; }
    void setColor(glm::vec3 const& color) { _color = color; }
    void setScale(float const scale) { _scale = scale; }
    void setFlat(bool const is_flat = true) { _is_flat = is_flat; }

    bool const isFlat() const { return _is_flat; }

  private:
    Renderer* _renderer;
    TextureManager* _texture_manager;
    LightManager* _light_manager;
    FontManager* _font_manager;

    std::string _text{};
    glm::vec3 _position{};
    glm::vec3 _color{};
    float _scale{1.0f};
    bool _is_flat{true};
  };
}
