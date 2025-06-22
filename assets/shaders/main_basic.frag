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
  mat4 model;
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
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
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

layout(binding = 1) uniform sampler2D tex_sampler[TEXTURE_COUNT];

layout(binding = 2) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

layout(binding = 3) uniform samplerCubeShadow tex_shadow_sampler;

layout(binding = 4) uniform samplerCube cube_maps[1];

float InverseSquareAttenuation(vec3 l)
{
  float r = 1.0;
  float r_max = 5.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = (r * r) / (r_max * r_max - r * r);
  atten_const.y = (r_max * r_max);

  float r2 = dot(l, l);
  return max(atten_const.x * (atten_const.y / r2 - 1.0), 0.0001);
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

  float r2 = dot(l, l);
  return max(exp(r2 * atten_const.x) * atten_const.y - atten_const.z, 0.0001);
}

float SmoothAttenuation(vec3 l, float r_max)
{
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

  float r2 = dot(l, l);
  return max(r2 * atten_const.x * (sqrt(r2) * atten_const.y - 3.0) + 1.0, 0.0001);
}

float linear_to_sRGB(float color)
{
  if (color <= GAMMA_TRESHOLD) {
    return color * 12.92;
  } else {
    return pow((color * 1.055), 1/2.4) - 0.055;
  }
}

float ShadowCalculation(vec3 light_coord, Light l, float NdL)
{
  vec3 p = light_coord;
  float shadow_offset = 2.f/3200.f;
  vec2 depth_transform = vec2(l.projection[2][2], l.projection[2][3]);

  vec3 absq = abs(p);
  float mxy = max(absq.x, absq.y);
  float m = max(mxy, absq.z);

  float offset = shadow_offset * m;
  float dxy  = (mxy > absq.z) ? offset : 0.0;
  float dx  = (absq.x > absq.y) ? dxy : 0.0;
  vec2 oxy = vec2(offset - dx, dx);
  vec2 oyz = vec2(offset - dxy, dxy);

  vec3 limit = vec3(m, m, m);
  limit.xy -= oxy * (1.0 / 3200);
  limit.yz -= oyz * (1.0 / 3200);

  //float depth = depth_transform.x + depth_transform.y / m;
  float depth = length(p) / 50.0f;//far plane
  float light = texture(tex_shadow_sampler, vec4(p, depth));

  p.xy -= oxy;
  p.yz -= oyz;
  light += texture(tex_shadow_sampler, vec4(clamp(p.xyz, -limit, limit), depth));
  p.xy += oxy * 2.0;
  light += texture(tex_shadow_sampler, vec4(clamp(p.xyz, -limit, limit), depth));
  p.yz += oyz * 2.0;
  light += texture(tex_shadow_sampler, vec4(clamp(p.xyz, -limit, limit), depth));
  p.xy -= oxy * 2.0;
  light += texture(tex_shadow_sampler, vec4(clamp(p.xyz, -limit, limit), depth));

  light *= 0.2;

  return light;
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
  vec3 F = F0 + (F90 - F0) * pow(max(1.0 - NdH, 0.0001), 1/P);

  return F;
}

float ReflectionBounce(vec3 F0)
{
  return length((20.0 / 21.0) * F0 + (1.0 / 21.0));
}

float GGXDistribution(float NdH, float roughness)
{

  float gamma = 2.0;

  float a = roughness * roughness;
  float a2 = a * a;
  float tmp = ((NdH * NdH) * (a2 - 1.0) + 1.0);
  float D = a2 / (PI * tmp * tmp);

  return D;
}

float SmithGeometryGGX(float roughness, float theta)
{
  float a = roughness + 1.0;
  float k = (a * a) / 8.0;

  return theta / (theta * (1.0 - k) + k);
}

vec3 linear_to_hdr10(vec3 color, float white_point)
{
  // Convert Rec.709 to Rec.2020 color space to broaden the palette
  const mat3 from709to2020 = mat3(
      0.6274040, 0.3292820, 0.0433136,
      0.0690970, 0.9195400, 0.0113612,
      0.0163916, 0.0880132, 0.8955950
  );
  //color = from709to2020 * color;

  // Normalize HDR scene values ([0..>1] to [0..1]) for ST.2084 curve
  const float st2084_max = 10000.0;
  color *= white_point / st2084_max;

  // Apply ST.2084 (PQ curve) for HDR10 standard
  const float m1 = 2610.0 / 4096.0 / 4.0;
  const float m2 = 2523.0 / 4096.0 * 128.0;
  const float c1 = 3424.0 / 4096.0;
  const float c2 = 2413.0 / 4096.0 * 32.0;
  const float c3 = 2392.0 / 4096.0 * 32.0;
  vec3 cp = pow(abs(color), vec3(m1));
  color = pow((c1 + c2 * cp) / (1.0 + c3 * cp), vec3(m2));

  return color;
}

vec3 srgb_to_linear(vec3 color)
{ 
  float gamma        = 2.4f; // The sRGB curve for mid tones to high lights resembles a gamma of 2.4
  vec3 linear_low  = color / 12.92;
  vec3 linear_high = pow((color + 0.055) / 1.055, vec3(gamma));
  vec3 is_high     = step(0.0404482362771082, color);
  return mix(linear_low, linear_high, is_high);
}

vec3 Diffuse(vec3 diffuse, vec3 F0, float NdL, float NdV)
{
  float a = 1.0 - pow(1.0 - max(NdL, 0.0001), 5);
  float b = 1.0 - pow(1.0 - max(NdV, 0.0001), 5);

  return (21.0 / (20.0 * PI)) * (1.0 - F0) * diffuse * a * b;
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

  vec2 xy = texture(tex_sampler[NORMAL_INDEX], normal_coord).rg * 2.0 - 1.0;
  //xy.y = -xy.y;
  float z = sqrt(max(0.001, 1.0 - dot(xy, xy)));
  vec3 normal = normalize(vec3(xy, z));
  normal = normalize(var.TBN * normal);

  //@todo use bitmask
  ivec2 normal_size = textureSize(tex_sampler[NORMAL_INDEX], 0);
  if (normal_size.x <= 2.0) {
    normal = var.norm;
  }

  normal = normalize(normal);

  vec2 mr_coord = transform_uv(
    material.mr_translation,
    material.mr_scale,
    material.mr_rotation,
    var.texture_coord);

  vec3 metal_roughness = texture(tex_sampler[METAL_ROUGHNESS_INDEX], mr_coord).rgb;
  float metallic = metal_roughness.x * material.mre_factor.g;
  float roughness = metal_roughness.y * material.mre_factor.b;

  ivec2 mr_size = textureSize(tex_sampler[METAL_ROUGHNESS_INDEX], 0);
  if (mr_size.x <= 2.0) {
    //metallic = material.mre_factor.x;
    //roughness = material.mre_factor.y;
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
  if (ao_size.x < 2.0) {
    ao = 1.0;
  }
  ao *= material.strength.y;//occlusion strength

  vec2 diffuse_coord = transform_uv(
    material.diffuse_translation,
    material.diffuse_scale,
    material.diffuse_rotation,
    var.texture_coord);
  
  vec4 albedo = texture(tex_sampler[DIFFUSE_INDEX], diffuse_coord);
  //albedo.xyz = srgb_to_linear(albedo.xyz);
  albedo.xyz = albedo.xyz;

  float color_alpha = albedo.a;
  
  if (material.alpha.x == 1.0 && color_alpha < material.alpha.y) discard;
  if (material.base_color.a < material.alpha.y) discard;

  albedo *= material.base_color * var.color;
  ivec2 albedo_size = textureSize(tex_sampler[DIFFUSE_INDEX], 0);
  if (albedo_size.x == 1 && albedo_size.y == 1) {
    albedo = material.base_color * var.color;
  }

  vec3 p = var.frag_pos;
  vec3 v = normalize(var.view_pos - p);

  //vec3 irradiance = texture(cube_maps[ENVIRONMENT_MAP_INDEX], normal).rgb;
  vec3 F0 = mix(vec3(0.04), albedo.xyz, metallic);

  vec4 C_diffuse = albedo / PI;
  vec4 C_specular = material.specular;
  
  //@todo looks good but is it ok?
  float P = material.mre_factor.y;// * (1.0 - roughness); 

  vec3 out_lights = vec3(0.0);

  vec3 F90 = vec3(1.0);

  //directional sun light
  //vec3 l = normalize(-sun_light.direction);
  float d = length(sun_light.position - p) * length(sun_light.position - p);
  //float d = 100000.0;//sun distance approx ?
  float radius = d * tan(0.00463);//sun approx angle
  vec3 l = normalize((sun_light.position - p) / d);
  vec3 h = normalize(l + v);
  float NdL = max(dot(normal, l), 0.00001);
  float HdV = max(dot(h, v), 0.00001);
  float NdV = max(dot(normal, v), 0.00001);
  float NdH = max(dot(normal, h), 0.00001);

  float f = ReflectionBounce(F0);
  vec3 F = FresnelSchlick(F0, F90, HdV, P) * f;
  float sun_roughness = roughness + (radius / (2 * d));
  float D = GGXDistribution(NdH, roughness);
  float G1 = SmithGeometryGGX(roughness, NdV);
  float G2 = SmithGeometryGGX(roughness, NdL);
  float G = G1 * G2;

  vec3 kD = vec3(1.0) - F;
  kD *= 1.0 - metallic;

  vec3 specular = (D * G * F) / ((4.0 * NdL * NdV));
  //vec3 radiance = srgb_to_linear(sun_light.color.rgb) * ao;
  vec3 diffuse = C_diffuse.xyz + Diffuse(C_diffuse.xyz, F0, NdL, NdV);

  vec3 radiance = (((radius * radius) / d) + 0.0001) * vec3(sun_light.color);
  radiance *= ((NdL + 1.0) / 2.0) * ((NdL + 1.0) / 2.0);

   //vec3 radiance = vec3(1.0) * ao;
  vec3 C_sun = (kD * diffuse + specular) * radiance * 0.01;
  vec3 C_ambient = albedo.xyz * 0.01;

  for (int i = 1; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = point_lights[i].position;
    vec3 l = normalize(light_pos - p);

    //@todo check thoses attenuation functions
    //float d = length(light_pos - p);
    float attenuation = SmoothAttenuation(l, 1.5);
    //float attenuation = 1.0 / (point_lights[i].clq.x + point_lights[i].clq.y * d + point_lights[i].clq.z * (d * d));
    //vec3 C_light = srgb_to_linear(point_lights[i].color.rgb) * attenuation;
    vec3 C_light = point_lights[i].color.rgb * attenuation * 2.0;
   
    vec3 h = normalize(l + v);
    float NdL = max(dot(normal, l), 0.00001);
    float NdH = max(dot(normal, h), 0.00001);
    float NdV = max(dot(normal, v), 0.00001);
    float HdV = max(dot(h, v), 0.00001);

    float f = ReflectionBounce(F0);
    vec3 F = FresnelSchlick(F0, F90, HdV, P) * f;

    float D = GGXDistribution(NdH, roughness);

    float G1 = SmithGeometryGGX(roughness, NdV);
    float G2 = SmithGeometryGGX(roughness, NdL);

    float G = G1 * G2;

    vec3 specular = (D * G * F) / ((4.0 * NdV * NdL));

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    vec3 diffuse = C_diffuse.xyz + Diffuse(C_diffuse.xyz, F0, NdL, NdV);

    out_lights += (kD * diffuse + specular) * C_light * NdL;
  }

  vec4 color = vec4(C_ambient + C_sun + out_lights, color_alpha);

  vec3 frag_to_light = p - point_lights[1].position;
  float shadow = ShadowCalculation(frag_to_light, point_lights[1], NdL);
  float shadowFactor = max(shadow, 0.1);
  color.xyz *= shadowFactor;
  color.xyz *= ao;
  color.xyz *= PI;

  vec2 emissive_coord = transform_uv(
    material.emissive_translation,
    material.emissive_scale,
    material.emissive_rotation,
    var.texture_coord);

  vec4 emissive_color = texture(tex_sampler[EMISSIVE_INDEX], emissive_coord);
  //emissive_color.xyz = srgb_to_linear(emissive_color.xyz);
  emissive_color.xyz = emissive_color.xyz;

  ivec2 emissive_color_size = textureSize(tex_sampler[EMISSIVE_INDEX], 0);
  if (emissive_color_size.x != 1 && emissive_color_size.y != 1) {
    color += material.mre_factor.z * emissive_color;
  }
  color += color * (material.mre_factor.z * material.emissive_color);

  float exposure = 1.0;
  color.rgb = vec3(1.0) - exp(-color.rgb * exposure);
//
//  color.r = linear_to_sRGB(color.r);
//  color.g = linear_to_sRGB(color.g);
//  color.b = linear_to_sRGB(color.b);

  //final_color = color;
  //@todo check how to get the precise value
  float white_point = 350;
  final_color = vec4(0.0, 0.0, 0.0, 1.0);
  final_color.rgb = linear_to_hdr10(color.rgb, white_point);

  if (material.alpha.x == 2.0) {
    final_color = vec4(linear_to_hdr10(color.rgb, white_point), color_alpha);
  }
}