#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define TEXTURE_COUNT 8

//texture map index
#define DIFFUSE_INDEX 0
#define ALPHA_INDEX 1
#define NORMAL_INDEX 2
#define SPECULAR_INDEX 3
#define METAL_ROUGHNESS_INDEX 4
#define EMISSIVE_INDEX 5
#define AO_INDEX 6
#define TRANSMISSION_INDEX 7

//depth / shadow map index
#define SHADOW_MAP_INDEX 0

//cube maps index
#define ENVIRONMENT_MAP_INDEX 0

#define SPEC_HIGHLIGHT_THRESHOLD 0.02

//constants
#define PI 3.141592653589793238462643383279
#define INV_PI 1/PI

#define GAMMA_TRESHOLD 0.0031308
#define NR_POINT_LIGHTS 2

layout(location = 0) out vec4 final_color;

layout(location = 0) in FRAG_VAR {
  vec3 frag_pos;
  vec3 view_pos;
  vec2 texture_coord;
  vec3 norm;
  vec4 color;
  mat3 TBN;
  vec4 light_space;
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
  vec4 base_color;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec3 transmittance;
  vec4 emission;
  vec3 shi_ior_diss; //shininess, ior, diss
  vec3 alpha;
  vec3 mr_factor;
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
  vec3 strength;//x: normal strength, y occlusion strength, z transmission strength
};

layout(binding = 1) uniform sampler2D tex_sampler[TEXTURE_COUNT];

layout(binding = 2) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

layout(binding = 3) uniform sampler2DShadow tex_shadow_sampler[1];

layout(binding = 4) uniform samplerCube cube_maps[1];

float InverseSquareAttenuation(vec3 l)
{
  float r = 1.0;
  float r_max = 5.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = (r * r) / (r_max * r_max - r * r);
  atten_const.y = (r_max * r_max);

  vec3 l_dir = abs(l);
  float r2 = dot(l_dir, l_dir);
  return max(atten_const.x * (atten_const.y / r2 - 1.0), 0.0);
}

float ExponentialAttenuation(vec3 l)
{
  float k = 2.0;
  float r_max = 10.0;
  vec3 atten_const = vec3(1.0);
  float neg_k_square = -(k * k);
  float exp_k_neg_square = exp(neg_k_square);
  atten_const.x = neg_k_square / (r_max * r_max);
  atten_const.y = 1.0 / (1.0 - exp_k_neg_square);
  atten_const.z = exp_k_neg_square / (1.0 - exp_k_neg_square);

  vec3 l_dir = abs(l);
  float r2 = dot(l_dir, l_dir);
  return max(exp(r2 * atten_const.x) * atten_const.y - atten_const.z, 0.0);
}

float SmoothAttenuation(vec3 l)
{
  float r_max = 5.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

  vec3 l_dir = abs(l);
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

float ShadowCalculation(vec4 light_space, float NdL)
{
  vec3 p = light_space.xyz / light_space.w;
  //p = p * 0.5 + 0.5;

  float light = texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p);
  
  ivec2 size = textureSize(tex_shadow_sampler[SHADOW_MAP_INDEX], 0);
  float shadow_xoffset = 1.0 / size.x;
  float shadow_yoffset = 1.0 / size.y;
  float bias = max(0.05 * (1.0 - NdL), 0.005);
  
  p.x -= shadow_xoffset;
  p.y -= shadow_yoffset;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
  p.x += shadow_xoffset * 2.0;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
  p.y += shadow_yoffset * 2.0;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
  p.x += shadow_xoffset * 2.0;
  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);

  light *= 0.2;

  return 1.0+light;
}

//https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md
vec2 transform_uv(vec3 t, vec3 s, vec3 r, vec2 c)
{
  if (t.z < 0.5) return c;//no transform needed

  mat3 translation = mat3(1,0,0, 0,1,0, t.x, -t.y, 1);
  mat3 rotation = mat3(
      cos(r.x), sin(r.x), 0,
     -sin(r.x), cos(r.x), 0,
      0, 0, 1);
  mat3 scale = mat3(s.x, 0, 0, 0, s.y, 0, 0, 0, 1);
  mat3 matrix = mat3(1.0);
  matrix *= translation * rotation * scale;
  vec2 uvTransformed = (matrix * vec3(c.xy, 1)).xy;

  return uvTransformed;
}

vec3 FresnelSchlick(vec3 F0, vec3 F90, float NdH, float P)
{
  vec3 bounce = (20.0 / 21.0) * F0 + (1.0 / 21.0);
  vec3 F = bounce + (F90 - bounce) * pow(max(1.0 - NdH, 0.0), 5);

  return F;
}

float GGXDistribution(float NdH, float roughness)
{
  float a = roughness * roughness;
  float a2 = a * a;
  float tmp = ((NdH * NdH) * (a - 1.0) + 1.0);
  float D = a / (PI * tmp * tmp);

  return D;
}

float SmithGeometryGGX(float roughness, float theta)
{
  float a = roughness + 1.0;
  float k = (a * a) / 8.0;

  return theta / (theta * (1.0 - k) + k);
}

void main()
{
  vec4 alpha_color = texture(tex_sampler[ALPHA_INDEX], var.texture_coord);
  ivec2 alpha_size = textureSize(tex_sampler[ALPHA_INDEX], 0);

  if (alpha_size.x != 1 && alpha_size.y != 1) {
    if (material.alpha.x == 1.0 && alpha_color.r < material.alpha.y) discard;
  }
 
  vec2 normal_coord = transform_uv(
    material.normal_translation,
    material.normal_scale,
    material.normal_rotation,
    var.texture_coord); 

  vec2 xy = texture(tex_sampler[NORMAL_INDEX], normal_coord).xy;
  xy = xy.xy * 2.0 - 1.0;
  xy.y = -xy.y;
  float z = clamp(sqrt(1 - dot(xy, xy)), 0.0, 1.0);
  vec3 normal = vec3(xy, z);
  normal = var.TBN * normal;

  ivec2 normal_size = textureSize(tex_sampler[NORMAL_INDEX], 0);
  if (normal_size.x <= 2.0) {
    normal = var.norm;
  }

  normal = normalize(normal * material.strength.x);

  vec2 mr_coord = transform_uv(
    material.mr_translation,
    material.mr_scale,
    material.mr_rotation,
    var.texture_coord);

  vec3 metal_roughness = texture(tex_sampler[METAL_ROUGHNESS_INDEX], mr_coord).rgb;
  float metallic = metal_roughness.x * material.mr_factor.b;
  float roughness = metal_roughness.y * material.mr_factor.g;

  ivec2 mr_size = textureSize(tex_sampler[METAL_ROUGHNESS_INDEX], 0);
  if (mr_size.x <= 2.0) {
    metallic = material.mr_factor.x;
    roughness = material.mr_factor.y;
  }

  vec2 transmission_coord = transform_uv(
    material.transmission_translation,
    material.transmission_scale,
    material.transmission_rotation,
    var.texture_coord);

  vec2 transmission = texture(tex_sampler[TRANSMISSION_INDEX], transmission_coord).bg;
  transmission.xy *= material.strength.z;

  ivec2 transmission_size = textureSize(tex_sampler[TRANSMISSION_INDEX], 0);
  if (transmission_size.x <= 2.0) {
    transmission = vec2(1.0) * material.strength.z;
  }
  
  float ao = texture(tex_sampler[AO_INDEX], var.texture_coord).r;
  ivec2 ao_size = textureSize(tex_sampler[AO_INDEX], 0);
  if (ao_size.x == 1 && ao_size.y == 1) {
    ao = 1.0;
  }
  ao *= material.strength.y;//occlusion strength

  vec2 diffuse_coord = transform_uv(
    material.diffuse_translation,
    material.diffuse_scale,
    material.diffuse_rotation,
    var.texture_coord);
  
  vec4 albedo = texture(tex_sampler[DIFFUSE_INDEX], diffuse_coord);
  float color_alpha = albedo.a;
  
  if (material.alpha.x == 1.0 && color_alpha < material.alpha.y) discard;
  if (material.base_color.a < material.shi_ior_diss.z) discard;

  albedo *= material.base_color * var.color;
  ivec2 albedo_size = textureSize(tex_sampler[DIFFUSE_INDEX], 0);
  if (albedo_size.x == 1 && albedo_size.y == 1) {
    albedo = material.base_color * var.color;
  }
  
  vec3 p = var.frag_pos;
  vec3 v = normalize(var.view_pos - p);

  vec3 irradiance = texture(cube_maps[ENVIRONMENT_MAP_INDEX], var.norm).rgb;
  vec3 F0 = mix(vec3(0.04), albedo.xyz, metallic);

  vec4 C_diffuse = albedo / PI;
  vec4 C_specular = material.specular;
  
  float P = 5.0 * (1.0 - roughness); 

  vec3 out_lights = vec3(0.0);

  vec3 F90 = vec3(1.0);
  //if (metallic >= 0.95) F90 = 50.0 * F0;
  
  for (int i = 1; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = point_lights[i].position;
    vec3 l = normalize(light_pos - p);

    //@todo check thoses attenuation functions
    float attenuation = SmoothAttenuation(l);
    vec3 l_color = point_lights[i].color;
    vec4 C_light = vec4(l_color, 1.0) * attenuation;
   
    vec3 h = normalize(l + v);
    float NdL = max(dot(normal, l), 0.0);
    float NdH = max(dot(normal, h), 0.0);
    float NdV = max(dot(normal, v), 0.0);
    float HdV = max(dot(h, v), 0.0);

    vec3 F = FresnelSchlick(F0, F90, HdV, P);

    float D = GGXDistribution(NdH, roughness);

    float G1 = SmithGeometryGGX(roughness, NdV);
    float G2 = SmithGeometryGGX(roughness, NdL);

    float G = G1 * G2;

    vec3 specular = (D * G * F) / (4.0 * NdV * NdL + 0.0001);

    vec3 diffuse = C_diffuse.xyz * (1.0 - F);;

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    out_lights += (kD * C_diffuse.xyz + specular) * C_light.xyz * NdL;
  }
  
  //directional sun light
  vec3 l = normalize(-sun_light.direction);
  vec3 h = normalize(l + v);
  float NdL = max(dot(normal, l), 0.0);
  float HdV = max(dot(h, v), 0.0);
  float NdV = max(dot(normal, v), 0.0);
  float NdH = max(dot(normal, h), 0.0);

  vec3 F = FresnelSchlick(F0, F90, HdV, P);
  float D = GGXDistribution(NdH, roughness);
  float G1 = SmithGeometryGGX(roughness, NdV);
  float G2 = SmithGeometryGGX(roughness, NdL);
  float G = G1 * G2;
  vec3 diffuse = C_diffuse.xyz * (1.0 - F);
  vec3 kD = vec3(1.0) - F;
  kD *= 1.0 - metallic;

  vec3 specular = (D * G * F) / (4.0 * NdL * NdV + 0.0001);
  vec3 radiance = sun_light.color * ao;
  vec3 C_sun = (kD * C_diffuse.xyz + specular) * radiance * NdL;
  vec3 C_ambient = albedo.xyz * ao * 0.001;

  vec4 color = vec4(C_ambient + C_sun + out_lights, color_alpha);
  
  float shadow = ShadowCalculation(var.light_space, NdL);
  color.xyz *= shadow;
  color *= PI;

  vec2 emissive_coord = transform_uv(
    material.emissive_translation,
    material.emissive_scale,
    material.emissive_rotation,
    var.texture_coord); 

  vec4 emissive_color = texture(tex_sampler[EMISSIVE_INDEX], emissive_coord);
  ivec2 emissive_color_size = textureSize(tex_sampler[EMISSIVE_INDEX], 0);
  if (emissive_color_size.x != 1 && emissive_color_size.y != 1) {
    color += material.emission * emissive_color;
  }

  //color = color / (color + vec4(1.0));

  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);

  final_color = color;

  if (material.alpha.x == 2.0) {
    final_color = vec4(color.xyz, color_alpha);
  }
}
