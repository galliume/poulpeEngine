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

struct Material
{
  vec4 base_color;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec3 transmittance;
  vec3 shi_ior_diss; //shininess, ior, diss
  vec3 alpha;
  vec3 mre_factor;
  vec3 normal_translation;
  vec3 normal_scale;
  vec3 normal_rotation;
  vec3 ambient_translation;
  vec3 ambient_scale;
  vec3 ambient_rotation;
  vec3 diffuse_translation;
  vec3 diffuse_scale;
  vec3 diffuse_rotation;
  vec3 emissive_translation;
  vec3 emissive_scale;
  vec3 emissive_rotation;
  vec3 mr_translation;
  vec3 mr_scale;
  vec3 mr_rotation;
  vec3 transmission_translation;
  vec3 transmission_scale;
  vec3 transmission_rotation;
  vec3 strength;//x: normal strength, y occlusion strength
  vec4 emissive_color;
};

struct Light {
  mat4 light_space_matrix;
  mat4 projection;
  mat4 view;
  //ambiance diffuse specular
  vec3 ads;
  //constant, linear, quadratiq
  vec3 clq;
  //cutOff, outerCutoff, Blank
  vec3 coB;
  vec3 color;
  vec3 direction;
  vec3 position;
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
};

#define NR_POINT_LIGHTS 2

layout(set = 0, binding = 1) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
// layout(location = 2) in vec2 texture_coord;
// layout(location = 3) in vec4 tangent;
// layout(location = 4) in vec4 color;

layout(push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
  vec4 options;
} pc;

layout(location = 0) out FRAG_VAR {
  float far_plane;
  vec4 frag_position;
  vec3 light_position;
  mat4 light_space_matrix;
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
} var;

void main()
{
  Light light = point_lights[1];

  var.light_position = light.position;
  var.light_space_matrix = light.light_space_matrix;
  var.light_space_matrix_left = light.light_space_matrix_left;
  var.light_space_matrix_top = light.light_space_matrix_top;
  var.light_space_matrix_right = light.light_space_matrix_right;
  var.light_space_matrix_bottom = light.light_space_matrix_bottom;
  var.light_space_matrix_back = light.light_space_matrix_back;

  var.far_plane = 25.0f;

  //vec4 p = ubo.projection * pc.view * vec4(position, 1.0);
  //gl_Position = p.xyww;
  gl_Position = pc.view * ubo.model * vec4(position, 1.0);

  var.frag_position = gl_Position;
} 