#version 450
#extension GL_EXT_nonuniform_qualifier : enable
//#extension GL_ARB_shader_viewport_index_layer : enable
#extension GL_NV_viewport_array2 : enable

struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
  UBO ubo;
};

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

#define NR_POINT_LIGHTS 2

layout(set = 0, binding = 1) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
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
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 original_pos;
layout(location = 5) in vec2 texture_coord;
layout(location = 6) in ivec4 bone_ids;
layout(location = 7) in vec4 bone_weights;
layout(location = 8) in float total_weight;

void main()
{
  int face = gl_InstanceIndex;
  gl_Layer = face;

  Light light = sun_light;
  mat4 light_matrices[4] = mat4[4](
    light.light_space_matrix,          // slice 0
    light.light_space_matrix_left,     // slice 1
    light.light_space_matrix_top,      // slice 2
    light.light_space_matrix_right     // slice 3
  );

  gl_Position = light_matrices[face] * ubo.model * vec4(position, 1.0);
  gl_Position.z / gl_Position.w;
}