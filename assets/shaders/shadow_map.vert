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
  vec4 cascade_texel_sizes;
};

#define NR_POINT_LIGHTS 2

layout(set = 0, binding = 1) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 position;
layout(location = 3) in vec4 normal;
layout(location = 4) in vec2 texture_coord;
//layout(location = 6) in ivec4 bone_ids;
//layout(location = 7) in vec4 bone_weights;
//layout(location = 8) in float total_weight;
// layout(location = 2) in vec2 texture_coord;
// layout(location = 3) in vec4 tangent;
// layout(location = 4) in vec4 color;

layout(location = 0) out vec4 out_tangent;
layout(location = 1) out vec4 out_color;
layout(location = 2) out vec4 out_position;

layout(push_constant) uniform constants
{
  mat4 view;
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

void main()
{
  int face = gl_InstanceIndex;

  gl_Layer = face;

  out_tangent = tangent;
  out_color = color;
  out_position = ubo.model * position;
  
  Light light = point_lights[1];
  mat4 light_matrices[6] = mat4[6](
    light.light_space_matrix_right,  // face 0
    light.light_space_matrix_left,   // face 1
    light.light_space_matrix_top,    // face 2
    light.light_space_matrix_bottom, // face 3
    light.light_space_matrix,        // face 4 (front)
    light.light_space_matrix_back    // face 5
  );

  //vec4 p = ubo.projection * pc.view * position;
  //gl_Position = p.xyww;
  //gl_Position = pc.view * ubo.model * position;
  gl_Position =  light_matrices[face] * out_position;
  gl_Position.z / gl_Position.w;
} 