#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

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

#define NR_POINT_LIGHTS 2

layout(std430, binding = 1) readonly buffer LightBuffer {
    Light lights[];
} lightData;

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

void main()
{
  Light light = lightData.lights[1];

  float light_distance = length(position.xyz - light.position.xyz);
  light_distance = light_distance / 500.f;
  gl_FragDepth = light_distance;
}
