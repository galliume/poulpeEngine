#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

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

layout(location = 0) out vec4 position;

void main()
{
  Light light = point_lights[1];
  mat4 light_matrices[6] = mat4[6](
    light.light_space_matrix_right,  // face 0
    light.light_space_matrix_left,   // face 1
    light.light_space_matrix_top,    // face 2
    light.light_space_matrix_bottom, // face 3
    light.light_space_matrix,        // face 4 (front)
    light.light_space_matrix_back    // face 5
  );

  for(int face = 0; face < 6; ++face)
  {
    gl_Layer = face;
    for(int i = 0; i < 3; ++i)
    {
      position = gl_in[i].gl_Position;
      gl_Position = light_matrices[face] * position;
      EmitVertex();
    }
    EndPrimitive();
  }
}
