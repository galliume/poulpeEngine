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
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_view_position;
layout(location = 3) out mat3 out_TBN;
layout(location = 6) out vec3 out_normal;

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

  float A = 0.1f;
  float L = 12.0f;
  const float g = 9.81f;
  float w = 2.0f / L;
  //float w = sqrt(9.8 * ((2.0 * PI) / L));
  float S = 5.0f; 
  float t = pc.options.x;

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
  
  const int waves_count = 16;
  for (int i = 0; i < waves_count; i++) {
    //speed
    float phi = S * (2.0f / L);

    //direction
    seed = fract(sin(seed * 43758.5453 + i) * 43758.5453);
    float angle = seed * PI;
    vec2 D = vec2(cos(angle), sin(angle));

    //Wi(x, y, t) = Ai * sin(Di dot (x, y) * wi + t * phii);
    float X = (dot(D, p.xz) + previous_dx) * w + t * phi;
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

  p.y += 2.5+displacement.y;

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
  out_view_position = pc.view_position;

  gl_Position = ubo.projection * pc.view * ubo.model * p;
}