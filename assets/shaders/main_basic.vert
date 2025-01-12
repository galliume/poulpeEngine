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
layout(location = 4) in vec4 color;

layout(location = 0) out FRAG_VAR {
  vec3 frag_pos;
  vec3 light_pos;
  vec3 view_pos;
  vec3 t_frag_pos;
  vec3 t_light_dir;
  vec3 t_view_dir;
  vec3 t_plight_pos[NR_POINT_LIGHTS];
  vec2 texture_coord;
  vec3 norm;
  vec4 color;
  vec3 tangent;
  vec3 bitangent;
  mat3 TBN;
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
  vec3 base_color;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 transmittance;
  vec3 emission;
  vec3 shi_ior_diss; //shininess, ior, diss
  vec3 alpha;
  vec3 mr_factor;
  vec3 normal_translation;//z: 0 no translation 1.0 translation
  vec3 normal_scale; //z: 0 no scale 1.0 scale
  vec3 normal_rotation; //y: 0 no rotation 1.0 rotation
  vec3 diffuse_translation;//z: 0 no translation 1.0 translation
  vec3 diffuse_scale; //z: 0 no scale 1.0 scale
  vec3 diffuse_rotation; //y: 0 no rotation 1.0 rotation
  vec3 emissive_translation;//z: 0 no translation 1.0 translation
  vec3 emissive_scale; //z: 0 no scale 1.0 scale
  vec3 emissive_rotation; //y: 0 no rotation 1.0 rotation
  vec3 strength;//x: normal strength, y occlusion strength
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
  mat3 inversed_model = inverse(mat3(ubo.model));
  mat3 normal_matrix = transpose(inversed_model);
  vec3 norm = normal;
  //norm.z = sqrt(1 - dot(norm.xy, norm.xy));
  norm = normalize(normal_matrix * normal);
  float handedness = tangent.w;

  vec3 T = normalize(normal_matrix * tangent.xyz);
  vec3 N = normalize(norm);
  T = normalize(T - dot(T, N) * N);
  vec3 B = normalize(cross(N, T) * handedness);
  mat3 TBN = transpose(mat3(T, B, N));
  
  frag_var.frag_pos = position;
  frag_var.t_frag_pos = TBN * frag_var.frag_pos;

  frag_var.light_pos = sun_light.position;
  frag_var.view_pos = pc.view_position;

  frag_var.t_view_dir = TBN * (frag_var.view_pos - frag_var.frag_pos);
  frag_var.t_light_dir = TBN * (frag_var.light_pos - frag_var.frag_pos);
  
  frag_var.t_plight_pos[0] = TBN * (point_lights[0].position);
  frag_var.t_plight_pos[1] = TBN * (point_lights[1].position);
  
  // TangentSpaceVL(frag_var.frag_pos, frag_var.view_pos, frag_var.light_pos, norm, tangent, frag_var.t_view_dir, frag_var.t_light_dir);
  // TangentSpaceVL(frag_var.frag_pos, frag_var.view_pos, point_lights[0].position, norm, tangent, frag_var.t_pview_dir[0], frag_var.t_plight_dir[0]);
  // TangentSpaceVL(frag_var.frag_pos, frag_var.view_pos, point_lights[1].position, norm, tangent, frag_var.t_pview_dir[1], frag_var.t_plight_dir[1]);

  //frag_var.light_space = (ubo.projection * sun_light.view * vec4(frag_var.frag_pos, 1.0));
  frag_var.texture_coord = texture_coord;
  frag_var.norm = norm;
  frag_var.color = color;
  frag_var.tangent = T;
  frag_var.bitangent = B;
  frag_var.TBN = TBN;
  gl_Position = ubo.projection * pc.view * ubo.model * vec4(position, 1.0f);
} 
