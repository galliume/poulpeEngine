#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define NR_POINT_LIGHTS 2

struct Light {
    mat4 light_space_matrix;
    mat4 projection;
    mat4 view;
    mat4 light_space_matrix_left;
    mat4 light_space_matrix_top;
    mat4 light_space_matrix_right;
    mat4 light_space_matrix_bottom;
    mat4 light_space_matrix_back;
    mat4 cascade0;

    vec4 ads;
    vec4 clq;
    vec4 coB;
    vec4 color;
    vec4 direction;
    vec4 position;

    vec4 cascade_scale1;
    vec4 cascade_scale2;
    vec4 cascade_scale3;
    vec4 cascade_offset1;
    vec4 cascade_offset2;
    vec4 cascade_offset3;

    vec4 cascade_min_splits;
    vec4 cascade_max_splits;
    vec4 cascade_texel_sizes;
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
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 position;
layout(location = 3) in vec4 normal;
layout(location = 4) in vec2 texture_coord;
//layout(location = 6) in ivec4 bone_ids;
//layout(location = 7) in vec4 bone_weights;
//layout(location = 8) in float total_weight;

layout(std430, binding = 3) readonly buffer LightBuffer {
    Light lights[];
} lightData;

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec4 out_options;

void main()
{
  vec4 p = position;

  out_texture_coord = texture_coord;
  out_options = color;

  gl_Position = p;
}