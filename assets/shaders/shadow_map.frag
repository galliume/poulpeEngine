#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

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

layout(push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
  vec4 options;
} pc;

layout(location = 0) in vec4 position;

void main()
{
  Light light = point_lights[1];

  float light_distance = length(position.xyz - light.position);
  light_distance = light_distance / pc.options.x;
  gl_FragDepth = light_distance;
}
