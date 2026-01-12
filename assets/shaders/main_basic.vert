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
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 position;
layout(location = 3) in vec4 normal;
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
  mat4 sun_light;
  vec4 cascade_coord;
  float depth;
  vec3 blend;
  vec3 n;
} frag_var;

struct Light {
  mat4 light_space_matrix;
  mat4 projection;
  mat4 view;
  vec3 ads;
  vec3 clq;
  vec3 coB;
  vec3 color;
  vec3 direction;
  vec3 position;
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
  mat4 cascade0;
  vec3 cascade_scale1;
  vec3 cascade_scale2;
  vec3 cascade_scale3;
  vec3 cascade_offset1;
  vec3 cascade_offset2;
  vec3 cascade_offset3;
  vec4 cascade_min_splits;
  vec4 cascade_max_splits;
  vec4 cascade_texel_sizes;
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

layout(set = 0, binding = 2) readonly buffer ObjectBuffer {
  Material material;
};

layout(binding = 5) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

void main()
{
  mat3 normal_matrix = mat3(transpose(inverse(ubo.model)));
  //mat3 normal_matrix = mat3(ubo.model);

  vec3 T = normalize(normal_matrix * normalize(tangent.xyz));
  vec3 N = normalize(normal_matrix * normalize(normal.xyz));
  T = normalize(T - dot(T, N) * N);
  vec3 B = normalize(cross(N, T)) * tangent.w;
  mat3 TBN = mat3(T, B, N);

  frag_var.TBN = TBN;

  vec4 world_pos = ubo.model * position;
  vec4 local_space = position;

  frag_var.norm = N;
  frag_var.frag_pos = world_pos.xyz;
  frag_var.view_pos = vec3(pc.view_position) - world_pos.xyz;
  frag_var.sun_light = sun_light.view;
  frag_var.texture_coord = texture_coord;
  frag_var.color = color;
  vec4 view_pos = pc.view * world_pos;
  frag_var.depth = -view_pos.z;

  frag_var.cascade_coord = sun_light.cascade0 * world_pos;
  vec3 cascade0 = frag_var.cascade_coord.xyz / frag_var.cascade_coord.w;

  mat4 camera_inverse = inverse(pc.view);
  vec3 n = -camera_inverse[2].xyz;
  vec3 c = camera_inverse[3].xyz;

  float inv_z_dist0 = 1.0 / (sun_light.cascade_max_splits.x - sun_light.cascade_min_splits.y);
  float inv_z_dist1 = 1.0 / (sun_light.cascade_max_splits.y - sun_light.cascade_min_splits.z);
  float inv_z_dist2 = 1.0 / (sun_light.cascade_max_splits.z - sun_light.cascade_min_splits.w);

  vec4 d1 = (inv_z_dist0 * (vec4(n, (-dot(n, c) - sun_light.cascade_min_splits.y))));
  vec4 d2 = (inv_z_dist1 * (vec4(n, (-dot(n, c) - sun_light.cascade_min_splits.z))));
  vec4 d3 = (inv_z_dist2 * (vec4(n, (-dot(n, c) - sun_light.cascade_min_splits.w))));

  frag_var.blend = vec3(dot(d1, world_pos), dot(d2, world_pos), dot(d3, world_pos));

  gl_Position = ubo.projection * pc.view * world_pos;
} 