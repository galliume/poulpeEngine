module;

#include <chrono>
#include <glm/glm.hpp>
#include <string_view>
#include <volk.h>

export module Poulpe.Renderer:Text;

import :VulkanRenderer;
import :RendererComponent;

namespace Poulpe
{
  export class Text : public RendererComponentConcept
  {
  public:
    ~Text() override = default;
    void operator()(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info) override;
    
    void setText(std::string_view text) { _text = text; }
    void setPosition(glm::vec3 const& position) { _position = position; }
    void setColor(glm::vec3 const& color) { _color = color; }
    void setScale(float const scale) { _scale = scale; }
    void setFlat(bool const is_flat = true) { _is_flat = is_flat; }
  
  bool isFlat() const { return _is_flat; }
  
  private:
    void createDescriptorSet(
      Renderer *const renderer,
      ComponentRenderingInfo const& component_rendering_info);

    std::string _text{};
    glm::vec3 _position{};
    glm::vec3 _color{};
    float _scale{1.0f};
    bool _is_flat{true};
  };

}
