export module Engine.Renderer.Vulkan.Text;

import std;

import Engine.Component.Mesh;

import Engine.Core.GLM;
import Engine.Core.Volk;

import Engine.Renderer.RendererComponentTypes;
import Engine.Renderer.VulkanRenderer;

namespace Poulpe
{
  export class Text
  {
  public:
    void operator()(
      Renderer & renderer,
      Mesh & mesh,
      RendererContext const& render_context);

    void setText(std::string_view text) { _text = text; }
    void setPosition(glm::vec3 const& position) { _position = position; }
    void setColor(glm::vec3 const& color) { _color = glm::vec4(color, 1.0f); }
    void setScale(float const scale) { _scale = scale; }
    void setFlat(bool const is_flat = true) { _is_flat = is_flat; }

  bool isFlat() const { return _is_flat; }

      VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

  private:
    void createDescriptorSet(
    Renderer & renderer,
    Mesh & mesh,
    RendererContext const& render_context);

    std::string _text{};
    glm::vec3 _position{};
    glm::vec4 _color{};
    float _scale{1.0f};
    bool _is_flat{true};
  };
}
