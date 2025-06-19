#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;
 
struct UBO
{
  mat4 model;
  mat4 projection;
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

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
  UBO ubo;
};

layout(location = 0) in FRAG_VAR {
  vec3 light_pos;
  float far_plane;
  vec4 position;
} var[];

layout(location = 0) out OUT_VAR {
  vec3 light_pos;
  float far_plane;
  vec4 position;
} out_var;

void main()
{
  Light light = point_lights[1];

  for(int face = 0; face < 6; ++face)
  {
    gl_Layer = face;
    for(int i = 0; i < 3; ++i)
    {
      //@todo fix ?
      if (i == 0) {
        gl_Position = light.light_space_matrix * var[i].position;
      } else if (i == 1) {
        gl_Position = light.light_space_matrix_left * var[i].position;
      } else if (i == 2) {
        gl_Position = light.light_space_matrix_top * var[i].position;
      } else if (i == 3) {
        gl_Position = light.light_space_matrix_bottom * var[i].position;
      } else if (i == 4) {
        gl_Position = light.light_space_matrix_right * var[i].position;
      } else if (i == 5) {
        gl_Position = light.light_space_matrix_back * var[i].position;
      }
      out_var.position = gl_in[i].gl_Position;
      out_var.light_pos = var[i].light_pos;
      out_var.far_plane = var[i].far_plane;
      EmitVertex();
    }
    EndPrimitive();
  }
}
