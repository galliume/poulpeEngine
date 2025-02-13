#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(quads, fractional_odd_spacing, ccw) in;

#define PI 3.141592653589793238462643383279

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

layout(binding = 1) uniform sampler2D tex_sampler[1];

layout(location = 0) in vec2 in_texture_coord[];
layout(location = 1) in vec4 in_options[];

layout(location = 0) out vec2 out_texture_coord;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_position;
layout(location = 3) out vec3 out_view_position;
layout(location = 4) out mat3 out_inverse_model;

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

  p.y += 5.f;

  float A = 1.0f;
  float L = 10.0f;
  float w = 1.0f / L;
  float S = 4.0f;
  float t = pc.options.x;

  float A_factor = 0.82;
  float w_factor = 1.18;
  float t_factor = 1.07;

  vec2 gradient = vec2(0.0);
  //@todo improve seed
  float seed = PI;
  
  float previous_x = 0.0;

  for (int i = 0; i < 16; i++) {
    seed = fract(sin(seed * 43758.5453 + i) * 43758.5453);
    float angle = seed * 2.0f * PI;
    float phi = S * (2.0f / L);

    vec2 D = vec2(cos(angle), sin(angle));

    float DdP = dot(D, p.xz);
    DdP += previous_x * 0.5;
    float x = DdP * w + (t * t_factor) * phi;
    float e = exp(sin(x) - 1.0);
    float height = A * sin(e);

    p.y += height;

    float dHdx = -w * D.x * A * e * cos(x);
    float dHdz = -w * D.y * A * e * cos(x);  

    gradient += vec2(dHdx, dHdz);
    previous_x = dHdx;

    A *= A_factor;
    w *= w_factor;
    t_factor *= 1.07;
  }

  //gradient = normalize(gradient);
  vec3 bitangent = normalize(vec3(1.0, 0.0, gradient.x));
  vec3 tangent = normalize(vec3(0.0, 1.0, gradient.y));
  //out_normal = normalize(vec3(-gradient.x, -gradient.y, 1.0));

  out_inverse_model = inverse(mat3(ubo.model));
  out_position = out_inverse_model * p.xyz;
  out_normal = transpose(out_inverse_model) * cross(bitangent, tangent);
  out_view_position = out_inverse_model * pc.view_position;
  out_texture_coord = texCoord;

  gl_Position = ubo.projection * pc.view * ubo.model * p;
}