#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
  UBO ubo;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texture_coord;

layout(location = 0) out vec2 out_tex_coords;
layout(location = 1) out vec3 out_color;

void main()
{
  gl_Position = ubo.projection * vec4(position.xy, 0.0, 1.0);
  out_tex_coords = texture_coord;
  out_color = color;
}
