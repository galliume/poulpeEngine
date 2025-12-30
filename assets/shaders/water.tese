#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(quads, fractional_odd_spacing, ccw) in;

#define PI 3.141592653589793238462643383279
#define NR_POINT_LIGHTS 2

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
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

layout(binding = 1) uniform sampler2D tex_sampler[1];

layout(binding = 3) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(location = 0) in vec2 in_texture_coord[];
layout(location = 1) in vec4 in_options[];

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_view_position;
layout(location = 3) out mat3 out_TBN;
layout(location = 6) out vec3 out_normal;

// CSM outputs for the fragment shader
layout(location = 7) out float out_depth;
layout(location = 8) out vec4 out_cascade_coord;
layout(location = 12) out vec3 out_blend;

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

  vec4 p00 = gl_in[0].gl_Position;
  vec4 p01 = gl_in[1].gl_Position;
  vec4 p10 = gl_in[2].gl_Position;
  vec4 p11 = gl_in[3].gl_Position;

  vec4 uVec = p01 - p00;
  vec4 vVec = p10 - p00;
  vec4 normal = normalize(vec4(cross(vVec.xyz, uVec.xyz), 0));

  vec4 p0 = (p01 - p00) * u + p00;
  vec4 p1 = (p11 - p10) * u + p10;
  vec4 p = (p1 - p0) * v + p0;

  //p.y += 5.f;

  float L = 12.0f;
  float A = 0.01 * L;
  const float g = 9.81f;
  float w = 2.0f / L;
  float w_pic = 0.1f;
  float L_pic = w_pic / 2.0f;
  float A_pic = 0.1f * L_pic;
  //float w = sqrt(9.8 * ((2.0 * PI) / L));
  float S = 5.0f; //m/s
  float t = pc.options;

  vec3 bi = vec3(0.0);
  vec3 ta = vec3(0.0);
  vec3 n = vec3(0.0);

  //@todo improve seed
  float seed = PI;
  float previous_dx = 0.0;
  vec4 displacement = vec4(0.0, 0.0, 0.0, 1.0);

  float steepness = 2.5;
  float steepness_factor = 0.66;
  float A_factor = 0.82;
  float w_factor = 1.18;
  float l_factor = 0.75;

  const int waves_count = 16;
  for (int i = 0; i < waves_count; i++) {
    //speed
    //S *= A/A_pic;
    float phi = S * (2.0f / L);

    //direction
    seed = fract(sin(seed * 43758.5453 + i) * 43758.5453);
    float angle = seed * 2.0f*PI;
    vec2 D = vec2(cos(angle), sin(angle));

    //phase
    float phase = seed * (2.0 * PI);

    //Wi(x, y, t) = Ai * sin(Di dot (x, y) * wi + t * phii);
    float X = (dot(D, p.xz)) * w + t * phi + phase;
    float W = A * sin(X);

    float WA = w * A;
    float S = sin(w * dot(D, p.xz) + t * phi);
    float C = cos(w * dot(D, p.xz) + t * phi);

    //bitangent
    bi.x += steepness * (D.x * D.x) * WA * S;
    bi.z += steepness * (D.x * D.y) * WA * S;
    bi.y += D.x * WA * C;

    //tangent
    ta.x += steepness * (D.x * D.y) * WA * S;
    ta.z += steepness * (D.y * D.y) * WA * S;
    ta.y += D.y * WA * C;

    //normal
    n.x += D.x * WA * C;
    n.z += D.y * WA * C;
    n.y += steepness * WA * S;

    displacement.x += (steepness * A * D.x * cos(X));
    displacement.z += (steepness * A * D.y * cos(X));
    displacement.y += W;

    previous_dx = displacement.x;

    A *= A_factor;
    w *= w_factor;
    steepness *= clamp(steepness_factor, 0.0, 1.0);
  }

  p.y += 2.0f + displacement.y;

  mat4 trans_model = transpose(inverse(ubo.model));
  vec3 bitangent = normalize(trans_model * vec4(1.0 - bi.x, bi.y, -bi.z, 1.0)).xyz;
  vec3 tangent = normalize(trans_model * vec4(-ta.x, ta.y, 1.0 - ta.z, 1.0)).xyz;
  vec3 N = normalize(trans_model * vec4(-n.x, 1.0 - n.y, -n.z, 1.0)).xyz;
  //vec3 N = normalize(cross(tangent, bitangent));

  mat3 TBN = mat3(tangent, bitangent, N);

  out_TBN = TBN;
  out_position = (ubo.model * vec4(p.xyz, 1.0)).xyz;
  out_normal = N;
  out_texture_coord = texCoord;
  out_view_position = vec3(pc.view_position) - out_position;

  vec4 world_pos = ubo.model * p;
  vec4 view_pos = pc.view * world_pos;
  out_depth = -view_pos.z;

  out_cascade_coord = sun_light.cascade0 * world_pos;

  mat4 camera_inverse = inverse(pc.view);
  vec3 nI = -camera_inverse[2].xyz;
  vec3 c = camera_inverse[3].xyz;

  float inv_z_dist0 = 1.0 / (sun_light.cascade_max_splits.x - sun_light.cascade_min_splits.y);
  float inv_z_dist1 = 1.0 / (sun_light.cascade_max_splits.y - sun_light.cascade_min_splits.z);
  float inv_z_dist2 = 1.0 / (sun_light.cascade_max_splits.z - sun_light.cascade_min_splits.w);

  vec4 d1 = (inv_z_dist0 * (vec4(nI, (-dot(nI, c) - sun_light.cascade_min_splits.y))));
  vec4 d2 = (inv_z_dist1 * (vec4(nI, (-dot(nI, c) - sun_light.cascade_min_splits.z))));
  vec4 d3 = (inv_z_dist2 * (vec4(nI, (-dot(nI, c) - sun_light.cascade_min_splits.w))));

  out_blend = vec3(dot(d1, world_pos),  dot(d2, world_pos), dot(d3, world_pos));

  gl_Position = ubo.projection * view_pos;
}