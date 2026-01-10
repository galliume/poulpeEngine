#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_UBOS 1000
#define NR_POINT_LIGHTS 2

struct UBO
{
    mat4 model;
    mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(push_constant) uniform constants
{
  mat4 view;
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec2 texture_coord;
//layout(location = 6) in ivec4 bone_ids;
//layout(location = 7) in vec4 bone_weights;
//layout(location = 8) in float total_weight;

layout(location = 0) out FRAG_VAR {
  vec3 frag_pos;
  vec3 view_pos;
  vec2 texture_coord;
  vec3 norm;
  vec4 color;
  mat3 TBN;
  vec4 light_space;
  mat4 model;
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
    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * ubos[gl_InstanceIndex].model * vec4(position, 1.0);

    vec3 t = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(tangent.xyz, 0.0)));
    vec3 n = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(normal, 0.0)));
    vec3 b = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(cross(n, t) * tangent.w, 0.0)));
    mat3 TBN = transpose(mat3(t, b, n));
    frag_var.TBN = TBN;

    frag_var.norm = normal;
    //fNormal = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
    frag_var.frag_pos = vec3(ubos[gl_InstanceIndex].model * vec4(position, 1.0));
    frag_var.texture_coord = texture_coord;
    frag_var.view_pos = vec3(pc.view_position);
} 