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
  vec4 view_position;
  vec4 total_position;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 fidtidBB;
layout(location = 5) in vec4 color;

layout(location = 0) out FRAG_VAR {
  mat3 TBN;
  vec4 sun_light_space;
  vec4 view_position;
  vec4 color;
  vec3 normal;
  vec3 position;
  vec3 tangent_light_pos;
  vec3 tangent_frag_pos;
  vec3 tangent_view_pos;
  vec2 texture_coord;
  flat int texture_ID;
  mat4 model;
  vec4 tangent;
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

void main()
{
  vec3 norm = normal;
  norm.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);
  norm = normalize(norm);
  
  mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));

  vec3 t = normalize(normal_matrix * tangent.xyz);
  vec3 n = normalize(normal_matrix * norm);
  t = normalize(t - dot(t, n) * n);
  vec3 b = cross(n, t);
  mat3 TBN = transpose(mat3(t, b, n));

  frag_var.TBN = TBN;
  frag_var.normal =  normal_matrix * norm;
  frag_var.position = vec3(ubo.model * vec4(position, 1.0));
  frag_var.texture_coord = texture_coord;
  frag_var.view_position = pc.view_position;
  frag_var.sun_light_space = (biasMat * sun_light.light_space_matrix * mat4(TBN)) * vec4(frag_var.position, 1.0);
  frag_var.color = color;
  frag_var.texture_ID = int(fidtidBB.y);
  frag_var.tangent_light_pos = TBN * sun_light.position;
  frag_var.tangent_view_pos = TBN * frag_var.view_position.xyz;
  frag_var.tangent_frag_pos = TBN * frag_var.position;
  frag_var.model = ubo.model;
  frag_var.tangent = tangent;
  
  gl_Position = ubo.projection * pc.view * vec4(frag_var.position, 1.0);
} 