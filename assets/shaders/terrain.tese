#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 2

layout(quads, fractional_odd_spacing, ccw) in;

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
  float cascade_texel_size;
};

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
  vec4 options;
} pc;

layout(binding = 1) uniform sampler2D tex_sampler[5];

layout(binding = 3) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(location = 0) in vec2 in_texture_coord[];

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec4 out_weights;
layout(location = 2) out vec4 out_normal;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_view_position;
layout(location = 5) out mat3 out_inverse_model;

// CSM outputs for the fragment shader
layout(location = 8) out float out_depth;
layout(location = 9) out vec4 out_cascade_coord;
layout(location = 13) out vec3 out_blend;

float attenuation(float min, float max, float x)
{
  float half_range = abs(max - min) * 0.5;
  float mid = min + half_range;
  float distance = abs(x - mid);

  float att = clamp((half_range - distance) / half_range, 0.0f, 1.0f);

  return att;
}

void main()
{  
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  vec2 t00 = in_texture_coord[0];
  vec2 t01 = in_texture_coord[1];
  vec2 t10 = in_texture_coord[2];
  vec2 t11 = in_texture_coord[3];

  vec2 t0 = (t01 - t00) * u + t00;
  vec2 t1 = (t11 - t10) * u + t10;
  vec2 texCoord = (t1 - t0) * v + t0;
  out_texture_coord = texCoord * 500.0;

  float height = texture(tex_sampler[0], texCoord).r;

  out_weights.x = attenuation(0.0f, 0.2f, height);
  out_weights.y = attenuation(0.15f, 0.3f, height);
  out_weights.z = attenuation(0.25f, 0.9f, height);
  out_weights.w = attenuation(0.85f, 1.0f, height);

  height *= 20.0f;

  vec4 p00 = gl_in[0].gl_Position;
  vec4 p01 = gl_in[1].gl_Position;
  vec4 p10 = gl_in[2].gl_Position;
  vec4 p11 = gl_in[3].gl_Position;

  vec4 uVec = p01 - p00;
  vec4 vVec = p10 - p00;
  vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

  vec4 p0 = (p01 - p00) * u + p00;
  vec4 p1 = (p11 - p10) * u + p10;
  vec4 p = (p1 - p0) * v + p0;
  p.y -= 2.5f;
  p += normal * height;

  vec4 world_pos = ubo.model * p;
  vec4 view_pos = pc.view * world_pos;

  out_normal = normal;
  out_position = world_pos.xyz;
  out_view_position = pc.view_position - out_position;
  out_inverse_model = inverse(mat3(ubo.model));
  out_depth = -view_pos.z;

  out_cascade_coord = sun_light.cascade0 * world_pos;

  mat4 camera_inverse = inverse(pc.view);
  vec3 n = -camera_inverse[2].xyz;
  vec3 c = camera_inverse[3].xyz;

  float inv_z_dist0 = 1.0 / (sun_light.cascade_max_splits.x - sun_light.cascade_min_splits.y);
  float inv_z_dist1 = 1.0 / (sun_light.cascade_max_splits.y - sun_light.cascade_min_splits.z);
  float inv_z_dist2 = 1.0 / (sun_light.cascade_max_splits.z - sun_light.cascade_min_splits.w);

  vec4 d1 = (inv_z_dist0 * (vec4(n, (-dot(n, c) - sun_light.cascade_min_splits.y))));
  vec4 d2 = (inv_z_dist1 * (vec4(n, (-dot(n, c) - sun_light.cascade_min_splits.z))));
  vec4 d3 = (inv_z_dist2 * (vec4(n, (-dot(n, c) - sun_light.cascade_min_splits.w))));

  out_blend = vec3(dot(d1, world_pos),  dot(d2, world_pos), dot(d3, world_pos));

  gl_Position = ubo.projection * view_pos;
}