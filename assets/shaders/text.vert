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
  vec3 view_position;
  vec4 options;
} pc;

layout(set = 0, binding = 0) uniform UniformBufferObject {
  UBO ubo;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texture_coord;

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec3 out_color;
layout(location = 2) out vec3 out_options;

void main()
{
  vec3 options = vec3(0.0, 0.0, 1.0);
  vec4 p = ubo.projection * vec4(position, 1.0);
  
  if (pc.options.x > 0.5) {
    options.x = 1.0;
    p = ubo.projection * pc.view * ubo.model * vec4(position, 1.0);
  }

  gl_Position = p;
  out_tex_coords = texture_coord;
  out_color = color;
  out_options = options;
}
