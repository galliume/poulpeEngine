#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
  UBO ubo;
};

layout(push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 weights;

layout(binding = 1) uniform sampler2D tex_sampler[1];

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec4 out_weights;
layout(location = 2) out vec3 out_position;

void main()
{
  float height = texture(tex_sampler[0], texture_coord).r;

  vec4 p = vec4(position, 1.0);

  out_texture_coord = texture_coord;
  out_weights = weights;
  out_position = position;

  gl_Position = ubo.projection * pc.view * p;
}