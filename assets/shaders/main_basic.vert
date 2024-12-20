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
layout(location = 4) in vec4 fidtidBB;
layout(location = 5) in vec4 color;

layout(location = 0) out FRAG_VAR {
  vec4 color;
  vec4 light_space;
  vec3 light_dir;
  vec3 ltan;
  vec3 normal;
  vec3 position;
  vec4 tangent;
  vec3 view_dir;
  vec3 view_position;
  vec3 vtan;
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
  mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));
  mat4 inversed_model = inverse(ubo.model);

  frag_var.position = (ubo.model * vec4(position, 1.0)).xyz;
  frag_var.color = color;
  frag_var.light_dir = sun_light.position - frag_var.position;
  frag_var.normal = normal_matrix * normal;
  frag_var.tangent = tangent;
  frag_var.texture_coord = texture_coord;
  frag_var.view_position = pc.view_position;
  frag_var.view_dir = frag_var.view_position - frag_var.position;
  frag_var.light_space = (biasMat * sun_light.light_space_matrix * ubo.model) * vec4(position, 1.0);

  //object-space
  vec3 pos = (inversed_model * vec4(position, 1.0)).xyz;
  vec3 camera_pos = (inversed_model * vec4(frag_var.view_position, 1.0)).xyz;
  vec3 light_pos = (inversed_model * vec4(sun_light.position, 1.0)).xyz;
  vec3 transformed_tangent = normal_matrix * tangent.xyz;
  vec4 t = vec4(transformed_tangent, tangent.w);
  vec3 n = frag_var.normal * normal_matrix;

  TangentSpaceVL(
    pos,
    camera_pos,
    light_pos,
    n,
    t,
    frag_var.vtan,
    frag_var.ltan);

  gl_Position = ubo.projection * pc.view * vec4(frag_var.position, 1.0);
} 