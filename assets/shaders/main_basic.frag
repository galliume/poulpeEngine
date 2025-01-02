#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

//texture map index
#define DIFFUSE_INDEX 0
#define ALPHA_INDEX 1
#define SPECULAR_INDEX 2
#define NORMAL_INDEX 3
#define METAL_ROUGHNESS_INDEX 4

//depth / shadow map index
#define SHADOW_MAP_INDEX 0

//constants
#define PI 3.1415926538
#define GAMMA_TRESHOLD 0.0031308
#define NR_POINT_LIGHTS 2

layout(location = 0) out vec4 final_color;

layout(location = 0) in FRAG_VAR {
  vec4 color;
  vec4 light_space;
  vec3 frag_pos;
  vec3 light_pos;
  vec3 view_pos;
  vec3 t_light_dir;
  vec3 t_view_dir;
  vec3 t_plight_dir[NR_POINT_LIGHTS];
  vec3 t_pview_dir[NR_POINT_LIGHTS];
  vec2 texture_coord;
} var;

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

layout(binding = 1) uniform sampler2D tex_sampler[5];

layout(binding = 2) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

layout(binding = 3) uniform sampler2DShadow tex_shadow_sampler[1];

float InverseSquareAttenuation(vec3 l_dir)
{
  float r = 1.0;
  float r_max = 5.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = (r * r) / (r_max * r_max - r * r);
  atten_const.y = (r_max * r_max);

  float r2 = dot(l_dir, l_dir);
  return max(atten_const.x * (atten_const.y / r2 - 1.0), 0.0);
}

float ExponentialAttenuation(vec3 p, vec3 l)
{
  float k = 2.0;
  float r_max = 30.0;
  vec3 atten_const = vec3(1.0);
  float neg_k_square = -(k * k);
  float exp_k_neg_square = exp(neg_k_square);
  atten_const.x = neg_k_square / (r_max * r_max);
  atten_const.y = 1.0 / (1.0 - exp_k_neg_square);
  atten_const.z = exp_k_neg_square / (1.0 - exp_k_neg_square);

  vec3 l_dir = abs(l - p);
  float r2 = dot(l_dir, l_dir);
  return max(exp(r2 * atten_const.x) * atten_const.y - atten_const.z, 0.0);
}

float SmoothAttenuation(vec3 p, vec3 l)
{
  float r_max = 15.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

  vec3 l_dir = l - p;
  float r2 = dot(l_dir, l_dir);
  return max(r2 * atten_const.x * (sqrt(r2) * atten_const.y - 3.0) + 1.0, 0.0);
}

float linear_to_sRGB(float color)
{
  if (color <= GAMMA_TRESHOLD) {
    return color * 12.92;
  } else {
    return pow((color * 1.055), 1/2.4) - 0.055;
  }
}

float ShadowCalculation(vec4 light_space, vec3 normal)
{
  vec3 p = light_space.xyz / light_space.w;
  p = p * 0.5 + 0.5;

  float light = texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p);

  ivec2 shadow_offset = textureSize(tex_shadow_sampler[SHADOW_MAP_INDEX], 0);
  float dx = 1.0 / float(shadow_offset.x);
  float dy = 1.0 / float(shadow_offset.y);
  float bias = max(0.05 * (1.0 - dot(normal, sun_light.direction)), 0.005);
  
  p.x -= dx;
  p.y -= dy;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
  p.x += dx * 2.0;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
  p.y += dy * 2.0;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
  p.x += dx * 2.0;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);

/*
  int range = 1;

  for (int x = -range; x <= range; x++)
  {
    for (int y = -range; y <= range; y++)
    {
      p.x += x * dx;
      p.y += y * dy;
      light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p);
    }
  }
  */
  light *= 0.2;

  return light;
}

void main()
{
  vec4 alpha_color = texture(tex_sampler[ALPHA_INDEX], var.texture_coord);
  ivec2 alpha_size = textureSize(tex_sampler[ALPHA_INDEX], 0);

  if (alpha_size.x != 1 && alpha_size.y != 1 && alpha_color.r < 0.1) {
    discard;
  }

  vec3 normal = normalize((texture(tex_sampler[NORMAL_INDEX], var.texture_coord).xyz - 0.5) * 2.0);

  vec3 p = var.frag_pos;
  vec3 v = var.view_pos;

  vec4 combined = texture(tex_sampler[METAL_ROUGHNESS_INDEX], var.texture_coord);
  float ao = combined.r;//ao ?
  float metallic = combined.b;
  float roughness = combined.g;

  vec4 C_diffuse = texture(tex_sampler[DIFFUSE_INDEX], var.texture_coord);

  if (C_diffuse.a < 0.1) discard;
  vec3 diffuse_color = C_diffuse.xyz;
  vec3 C_ambient = C_diffuse.xyz * 0.5;
  vec3 C_light = sun_light.color * 0.001;
  
  vec3 ambient = C_ambient * C_light;
  vec3 color = ambient;

  for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
   vec3 l_dir = abs(point_lights[i].position - p);
   float attenuation = InverseSquareAttenuation(l_dir);
   C_light = point_lights[i].color * attenuation;
   
   float NdL = max(dot(normal, normalize(-var.t_plight_dir[i])), 0.0);

   color += diffuse_color * C_light * NdL;

   //vec3 r = 2 * NdL * normal - l;

  }

  //color *= PI;

  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);
  
  final_color = vec4(color, 1.0);
}
