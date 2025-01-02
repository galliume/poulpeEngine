#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define NR_POINT_LIGHTS 2

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
  vec4 total_position;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 bitangent;
layout(location = 5) in vec4 fidtidBB;
layout(location = 6) in vec4 color;

layout(location = 0) out FRAG_VAR {
  vec4 color;
  vec4 light_space;
  vec3 frag_pos;
  vec3 light_pos;
  vec3 view_pos;
  vec3 t_light_dir;
  vec3 t_view_dir;
  vec3 t_plight_dir[NR_POINT_LIGHTS];
  vec3 t_pview_dir[NR_POINT_LIGHTS];
  vec2 texture_coord;
} frag_var;

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
};

struct Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 transmittance;
  vec3 emission;
  //shininess, ior, diss
  vec3 shi_ior_diss;
};

layout(set = 0, binding = 2) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

const mat4 biasMat = mat4(
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0);

void TangentSpaceVL(vec3 pos, vec3 camera_pos, vec3 light_pos, vec3 normal, vec4 tangent, out vec3 vtan, out vec3 ltan)
{
  vec3 bitangent = cross(normal, tangent.xyz) * tangent.w;
  vec3 v = camera_pos - pos;
  vec3 l = light_pos - pos;
  vtan = vec3(dot(tangent.xyz, v), dot(bitangent, v), dot(normal, v));
  ltan = vec3(dot(tangent.xyz, l), dot(bitangent, l), dot(normal, l));
}

void main()
{
  mat4 inversed_model = inverse(ubo.model);
  mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));
  vec3 norm = normal_matrix * normal;
  //norm.y *= -1.0;
  //norm *= normal_matrix;

  frag_var.frag_pos = mat3(ubo.model) * position;
  frag_var.light_pos = sun_light.position;
  frag_var.view_pos = pc.view_position;

  TangentSpaceVL(frag_var.frag_pos, frag_var.view_pos, frag_var.light_pos, norm, tangent, frag_var.t_view_dir, frag_var.t_light_dir);
  TangentSpaceVL(frag_var.frag_pos, frag_var.view_pos, point_lights[0].position, norm, tangent, frag_var.t_pview_dir[0], frag_var.t_plight_dir[0]);
  TangentSpaceVL(frag_var.frag_pos, frag_var.view_pos, point_lights[1].position, norm, tangent, frag_var.t_pview_dir[1], frag_var.t_plight_dir[1]);

  frag_var.color = color;
  frag_var.light_space = (ubo.projection * sun_light.view * vec4(frag_var.frag_pos, 1.0));
  frag_var.texture_coord = texture_coord;

  gl_Position = ubo.projection * pc.view * ubo.model * vec4(position, 1.0f);
} 