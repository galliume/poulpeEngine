#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(push_constant) uniform constants
{
  mat4 view;
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

layout(set = 0, binding = 0) uniform UniformBufferObject {
  UBO ubo;
};

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 original_pos;
layout(location = 5) in vec2 texture_coord;
layout(location = 6) in ivec4 bone_ids;
layout(location = 7) in vec4 bone_weights;
layout(location = 8) in float total_weight;

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec3 out_color;
layout(location = 2) out vec3 out_options;

void main()
{
  vec3 options = vec3(0.0, 0.0, 1.0);
  vec4 p = ubo.projection * vec4(position, 1.0);

  if (bool((pc.options >> 0u) & 1u)) {
    options.x = 1.0;
    p = ubo.projection * pc.view * ubo.model * vec4(position, 1.0);
  }

  gl_Position = p;
  out_tex_coords = texture_coord;
  out_color = color.rgb;
  out_options = options;
}
