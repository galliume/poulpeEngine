#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(quads, fractional_odd_spacing, ccw) in;

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
} pc;

layout(binding = 1) uniform sampler2D tex_sampler[5];

layout(location = 0) in vec2 in_texture_coord[];

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec4 out_weights;
layout(location = 2) out vec4 out_normal;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_view_position;
layout(location = 5) out mat3 out_inverse_model;

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

  out_weights.x = attenuation(-0.5f, 0.2f, height);
  out_weights.y = attenuation(0.1f, 0.3f, height);
  out_weights.z = attenuation(0.2f, 0.9f, height);
  out_weights.w = attenuation(0.7f, 1.2f, height);

  height *= 50.0f;

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
  p += normal * height;
  p.y -= 1.5f;

  out_normal = normal;
  out_position = mat3(ubo.model) * p.xyz;
  out_view_position = pc.view_position;
  out_inverse_model = inverse(mat3(ubo.model));

  gl_Position = ubo.projection * pc.view * ubo.model * p;
}