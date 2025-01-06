#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

//texture map index
#define DIFFUSE_INDEX 0
#define ALPHA_INDEX 1
#define NORMAL_INDEX 2
#define SPECULAR_INDEX 3
#define METAL_ROUGHNESS_INDEX 4
#define EMISSIVE_INDEX 5
#define AO_INDEX 6

//depth / shadow map index
#define SHADOW_MAP_INDEX 0

//constants
#define PI 3.141592653589793238462643383279
#define GAMMA_TRESHOLD 0.0031308
#define NR_POINT_LIGHTS 2

layout(location = 0) out vec4 final_color;

layout(location = 0) in FRAG_VAR {
  vec3 frag_pos;
  vec3 light_pos;
  vec3 view_pos;
  vec3 t_frag_pos;
  vec3 t_light_dir;
  vec3 t_view_dir;
  vec3 t_plight_pos[NR_POINT_LIGHTS]; 
  vec2 texture_coord;
  vec3 norm;
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

layout(binding = 1) uniform sampler2D tex_sampler[7];

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

  vec3 normal = vec3(1.0); 
  normal.xy = texture(tex_sampler[NORMAL_INDEX], var.texture_coord).ga;
  normal.xy = normal.xy * 0.5 - 0.5;
  normal.z = sqrt(1 - dot(normal.xy, normal.xy));
  normal = normalize(normal);

  vec3 p = normalize(var.t_frag_pos);
  vec3 v = normalize(var.t_view_dir);

  vec2 metal_roughness = texture(tex_sampler[METAL_ROUGHNESS_INDEX], var.texture_coord).rg;
  float metallic = metal_roughness.r;
  float roughness = metal_roughness.g;

  float ao = texture(tex_sampler[AO_INDEX], var.texture_coord).r;

  vec4 albedo = texture(tex_sampler[DIFFUSE_INDEX], var.texture_coord);
  //if (albedo.a < 0.1) discard;

  vec4 C_ambient = vec4(material.ambient, 1.0) * albedo;
  vec4 C_diffuse = vec4(material.diffuse, 1.0) * albedo / PI;
  vec4 C_light = vec4(sun_light.color * 0.1, 1.0);
  vec4 C_specular = vec4(material.specular, 1.0);

  vec4 color = C_ambient * C_light * ao;

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, C_diffuse.xyz, metallic);

  for (int i = 1; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = normalize(var.t_plight_pos[i]);
    vec3 l = normalize(light_pos - p);
    vec3 h = normalize(v + l);
    float r = max(dot(h, v), 0.0);

    float distance = length(point_lights[i].position - var.frag_pos);
    float attenuation = 1.0 / (distance * distance);
    //float attenuation = InverseSquareAttenuation(l);
    C_light = vec4(vec3(1.0), 1.0);

    //Fresnel
    //if F0 < 0.02, no reflectance ?
    vec3 F = F0 + (1 - F0) * pow(clamp(1.0 - r, 0.0, 1.0), 5.0);

    float NdL = max(dot(normal, l), 0.0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    color += (vec4(kD, 1.0) * C_diffuse) * C_light;
  }

  vec3 l = normalize(var.t_plight_pos[1] - p);
  float diff = max(dot(l, normal), 0.0);

  //color = (C_ambient * albedo * 0.2) + (C_diffuse * albedo * diff);
  //color = C_diffuse;
  color *= PI;

  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);

  final_color = vec4(color.xyz , 1.0);
}
