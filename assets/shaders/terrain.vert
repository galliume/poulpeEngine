#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define NR_POINT_LIGHTS 2

struct Light {
  mat4 light_space_matrix;
  mat4 projection;
  mat4 view;
  vec3 ads;
  vec3 clq;
  vec3 coB;
  vec3 color;
  vec3 direction;
  vec3 position;
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
  mat4 cascade0;
  vec3 cascade_scale1;
  vec3 cascade_scale2;
  vec3 cascade_scale3;
  vec3 cascade_offset1;
  vec3 cascade_offset2;
  vec3 cascade_offset3;
  vec4 cascade_min_splits;
  vec4 cascade_max_splits;
  float cascade_texel_size;
};

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
  vec4 options;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 color;

layout(binding = 1) uniform sampler2D tex_sampler[5];

layout(binding = 3) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec3 out_position;

void main()
{
  out_texture_coord = texture_coord;
  out_position = position;

  gl_Position = vec4(position, 1.0);
}