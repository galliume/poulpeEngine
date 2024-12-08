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
  vec4 tangent;
  vec3 light_dir;
  vec3 normal;
  vec3 position;
  vec3 view_dir;
  vec3 view_position;
  vec2 texture_coord;
  vec3 light_pos_model_space;
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

#define NORMAL_INDEX 3

layout(binding = 1) uniform sampler2D tex_sampler[4];

void main()
{
  mat3 normal_matrix = transpose(inverse(mat3(ubo.model)));
  mat4 inversed_model = inverse(ubo.model);

  vec3 n = normalize(normal);
  n.z = sqrt(1.0 - n.x * n.x - n.y * n.y);

  frag_var.color = color;
  frag_var.light_pos_model_space = (ubo.model * vec4(sun_light.position, 1.0)).xyz;
  frag_var.light_space = (ubo.projection * sun_light.view * vec4(frag_var.position, 1.0)); 
  frag_var.position = (ubo.model * vec4(position, 1.0)).xyz;
  frag_var.tangent = tangent;
  frag_var.texture_coord = texture_coord;
  frag_var.view_position = pc.view_position;

  vec3 view_dir = frag_var.view_position - frag_var.position;
  vec3 light_dir = sun_light.position - frag_var.position;

  //@todo get rid of if
  ivec2 tex_size = textureSize(tex_sampler[NORMAL_INDEX], 0);
  if (tex_size.x != 1 && tex_size.y != 1) {
    vec3 m = texture(tex_sampler[NORMAL_INDEX], texture_coord).xyz * 2.0 - 1.0;
    vec3 t = normalize(tangent.xyz - n * dot(tangent.xyz, n));
    vec3 b = cross(normal, tangent.xyz) * tangent.w;
    vec3 nn = normalize(vec3(t * m.x + b * m.y + n * m.z));
    nn.z = sqrt(1.0 - m.x * m.x - m.y * m.y);
    n = nn;
//    TangentSpaceVL(
//      position,
//      pc.view_position,
//      sun_light.position,
//      n,
//      tangent,
//      view_dir,
//      light_dir);
  }

  frag_var.normal = n;
  frag_var.view_dir = view_dir;
  frag_var.light_dir = light_dir;

  gl_Position = ubo.projection * pc.view * vec4(frag_var.position, 1.0);
} 