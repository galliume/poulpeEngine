#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define GAMMA_TRESHOLD 0.0031308
#define PI 3.141592653589793238462643383279
#define NR_POINT_LIGHTS 2

#define HEIGHT_MAP 0
#define TERRAIN_GROUND 1
#define TERRAIN_GRASS 2
#define TERRAIN_SNOW 3
#define TERRAIN_SAND 4
#define HI_NOISE 5
#define LOW_NOISE 6

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
  mat4 cascade_scale_offset;
  mat4 cascade_scale_offset1;
  mat4 cascade_scale_offset2;
  mat4 cascade_scale_offset3;
  vec4 cascade_min_splits;
  vec4 cascade_max_splits;
  vec4 cascade_texel_size;
};

layout(push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
  vec4 options;
} pc;

layout(location = 0) out vec4 final_color;

layout(location = 0) in vec2 in_texture_coord;
layout(location = 1) in vec4 in_weights;
layout(location = 2) in vec4 in_normal;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_view_position;
layout(location = 5) in mat3 in_inverse_model;

layout(location = 8) in float in_depth;
layout(location = 9) in vec4 in_cascade_coord;
layout(location = 10) in vec4 in_cascade_coord1;
layout(location = 11) in vec4 in_cascade_coord2;
layout(location = 12) in vec4 in_cascade_coord3;
layout(location = 13) in vec3 in_cascade_blend;

layout(binding = 1) uniform sampler2D tex_sampler[7];
layout(binding = 3) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};
layout(binding = 2) uniform samplerCube env_sampler[];
layout(binding = 4) uniform sampler2DArrayShadow csm;

float linear_to_sRGB(float color)
{
  if (color <= GAMMA_TRESHOLD) {
    return color * 12.92;
  } else {
    return pow((color * 1.055), 1/2.4) - 0.055;
  }
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

vec4 tex_color(int index)
{
  vec4 color = vec4(texture(tex_sampler[index], in_texture_coord).rgb, 1.0);
  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);

  return color;
}

vec3 FresnelSchlick(vec3 F0, vec3 F90, float NdH, float P)
{
  return F0 + (F90 - F0) * pow(clamp(1.0 - NdH, 0.0, 1.0), 5);
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

vec3 Diffuse(vec3 diffuse, vec3 F0, float NdL, float NdV)
{
  float a = 1.0 - pow(1.0 - max(NdL, 0.00001), 5);
  float b = 1.0 - pow(1.0 - max(NdV, 0.00001), 5);

  return (21.0 / (20.0 * PI)) * (1.0 - F0) * diffuse * a * b;
}

float SmoothAttenuation(vec3 l, float r_max)
{
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

  float r2 = dot(l, l);
  return max(r2 * atten_const.x * (sqrt(r2) * atten_const.y - 3.0) + 1.0, 0.0001);
}

//double turbulence(double x, double y, double z, double f)
//{
//  const int W = 256;
//
//  double t = -.5;
//  for (; f < = W / 12; f *= 2)
//    t += abs(noise(x, y, z, f) / f);
//  return t;
//}
vec3 ApplyFog(vec3 shadedColor, vec3 v)
{
  float fogDensity = 0.001;
  vec3 fogColor = vec3(0.0, 0.0, 0.1);

  float f = exp(-fogDensity * length(v));
  return (mix(fogColor, shadedColor, f));
}

float CalculateInfiniteShadow(vec3 cascade_coord0, vec3 cascade_blend, float NdL)
{
  vec3 cascade_coord1 = in_cascade_coord1.xyz / in_cascade_coord1.w;
  vec3 cascade_coord2 = in_cascade_coord2.xyz / in_cascade_coord2.w;
  vec3 cascade_coord3 = in_cascade_coord3.xyz / in_cascade_coord3.w;

  vec3 blend = clamp(cascade_blend, 0.0, 1.0);
  float cascade_index;
  if (in_depth < sun_light.cascade_max_splits.x) {
    cascade_index = 0.0;
  } else if (in_depth < sun_light.cascade_max_splits.y) {
    cascade_index = 1.0;
  } else if (in_depth < sun_light.cascade_max_splits.z) {
    cascade_index = 2.0;
  } else {
    cascade_index = 3.0;
  }
  float weight = 0.0;
  vec3 shadow_coord1, shadow_coord2;

  if (cascade_index == 0.0) { shadow_coord1 = cascade_coord0; shadow_coord2 = cascade_coord1; weight = blend.x; }
  if (cascade_index == 1.0) { shadow_coord1 = cascade_coord1; shadow_coord2 = cascade_coord2; weight = blend.y; }
  if (cascade_index == 2.0) { shadow_coord1 = cascade_coord2; shadow_coord2 = cascade_coord3; weight = blend.z; }
  if (cascade_index == 3.0) { shadow_coord1 = cascade_coord3; shadow_coord2 = cascade_coord3; weight = 1.0; }

  float max_bias = 0.005;
  float min_bias = 0.0005;
  float bias = max(max_bias * (1.0 - NdL), min_bias);

  float light1 = texture(csm, vec4(shadow_coord1.xy, cascade_index, shadow_coord1.z - bias));

  float light2 = light1;
  if (cascade_index < 3.0) {
    light2 = texture(csm, vec4(shadow_coord2.xy, cascade_index + 1.0, shadow_coord2.z - bias));
  }

  return mix(light1, light2, weight);
}

void main()
{
  vec4 albedo = vec4(0.0);

  // Texture splatting for albedo
  albedo += texture(tex_sampler[TERRAIN_SAND], in_texture_coord) * in_weights.x;
  albedo += texture(tex_sampler[TERRAIN_GROUND], in_texture_coord) * in_weights.y;
  albedo += texture(tex_sampler[TERRAIN_GRASS], in_texture_coord) * in_weights.z;
  albedo += texture(tex_sampler[TERRAIN_SNOW], in_texture_coord) * in_weights.w;

  vec3 p = in_position;
  vec3 v = normalize(pc.view_position - p);
  vec3 normal = normalize(in_normal.xyz);

  // PBR material properties for terrain (dielectric)
  float metallic = 0.0;
  float roughness = 0.9; // Terrain is generally very rough

  // Base reflectivity for a non-metal
  vec3 F0 = mix(vec3(0.04), albedo.xyz, metallic);
  float P = 1.0; // Fresnel power
  vec3 F90 = vec3(1.0);

  // Sun light calculation
  vec3 l = normalize(-sun_light.direction);
  vec3 h = normalize(l + v);
  float NdL = max(dot(normal, l), 0.00001);
  float HdV = max(dot(h, v), 0.00001);
  float NdV = max(dot(normal, v), 0.00001);
  float NdH = max(dot(normal, h), 0.00001);

  vec3 F = FresnelSchlick(F0, F90, HdV, P);
  float D = GGXDistribution(NdH, roughness);
  float G1 = SmithGeometryGGX(roughness, NdV);
  float G2 = SmithGeometryGGX(roughness, NdL);
  float G = G1 * G2;

  // Energy conservation for diffuse/specular
  vec3 kD = vec3(1.0) - F;
  kD *= 1.0 - metallic;

  vec3 specular = (D * G * F) / ((4.0 * NdL * NdV));
  vec3 diffuse = Diffuse(albedo.xyz, F0, NdL, NdV);

  vec3 radiance = sun_light.color.rgb;
  vec3 C_sun = (kD * diffuse + specular) * radiance * NdL;
  vec3 C_ambient = albedo.xyz * 0.03; // Basic ambient term

  // Point lights calculation
  vec3 out_lights = vec3(0.0);

  for (int i = 0; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = point_lights[i].position;
    vec3 l = normalize(light_pos - p);

    //@todo check thoses attenuation functions
    float d = length(light_pos - p);
    float attenuation = 1.0 / (d * d); // Physically correct inverse-square falloff
    vec3 C_light = point_lights[i].color.rgb * attenuation;

    vec3 h = normalize(l + v);
    float NdL = max(dot(normal, l), 0.00001);
    float NdH = max(dot(normal, h), 0.00001);
    float NdV = max(dot(normal, v), 0.00001);
    float HdV = max(dot(h, v), 0.00001);

    vec3 F = FresnelSchlick(F0, F90, HdV, P);
    float D = GGXDistribution(NdH, roughness);
    float G1 = SmithGeometryGGX(roughness, NdV);
    float G2 = SmithGeometryGGX(roughness, NdL);
    float G = G1 * G2;

    vec3 specular = (D * G * F) / ((4.0 * NdV * NdL));

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    vec3 diffuse = Diffuse(albedo.xyz, F0, NdL, NdV);

    out_lights += (kD * diffuse + specular) * C_light * NdL;
  }

  vec3 csm_coords = in_cascade_coord.xyz / in_cascade_coord.w;
  float csm_shadow = CalculateInfiniteShadow(csm_coords, in_cascade_blend, NdL);
  C_sun *= max(csm_shadow, 0.1);

  // vec3 light_color = vec3(1.0);
  // vec3 ambient = 0.1 * light_color;

  // //@todo a point lights...

  // //sun directionnal light
  // vec3 light_pos = vec3(0.0, 500.0, 0.0);

  // vec3 normal = normalize(in_normal.xyz);
  // //vec3 x = dFdx(in_position);
  // //vec3 y = dFdy(in_position);
  // //vec3 in_normal = in_inverse_model * normalize(cross(x, y));

  // vec3 p = in_position;
  // vec3 v = normalize(pc.view_position - p);
  // vec3 i = normalize(p - pc.view_position);

  // vec3 l = normalize(light_pos - p);
  // vec3 h = normalize(v + l);

  // float HdV = max(dot(h, v), 0.00001);
  // float NdH = max(dot(normal, h), 0.00001);
  // float NdL = max(dot(normal, l), 0.00001);
  // float NdV = max(dot(normal, v), 0.00001);

  // vec3 F0 = vec3(0.02);
  // vec3 F90 = vec3(1.0);
  // float metallic = 0.2;
  // float P = 1.0;
  // float roughness = 0.0;

  // float D = GGXDistribution(NdH, roughness);
  // float G1 = SmithGeometryGGX(roughness, NdV);
  // float G2 = SmithGeometryGGX(roughness, NdL);
  // float G = G1 * G2;

  // float f = ReflectionBounce(F0);
  // vec3 F = FresnelSchlick(F0, F90, HdV, P);

  // vec3 specular = (D * G * F) / (4.0 * NdV * NdL);
  // vec3 diffuse = color.rgb + Diffuse(color.rgb, F0, NdL, NdV);

  // vec3 kD = vec3(1.0) - F;
  // kD *= 1.0 - metallic;

  // //vec3 r = reflect(i, normalize(in_normal));
  // //vec3 env_color = vec3(texture(env_sampler[0], r).rgb);
  // //diffuse =  mix(env_color, diffuse, 1.0);

  // vec3 C_sun = (kD * diffuse + specular) * light_color * NdL;
  // vec3 C_ambient = vec3(0.03) * vec3(1.0);
  // vec3 result = (C_ambient + C_sun);

//  vec3 i = normalize(in_position - pc.view_position);
//
//  vec3 r = reflect(i, normalize(norm));
//  vec3 env_color = vec3(texture(env_sampler[0], r).rgb);
//
//  vec3 light_dir = normalize(light_pos - in_position);
//  float diff = max(dot(norm, light_dir), 0.0);
//  vec3 diffuse = diff * light_color;
//  diffuse =  mix(env_color, diffuse, 1.0);
//
//  vec3 view_dir = normalize(in_view_position - in_position);
//  vec3 reflect_dir = reflect(-light_dir, norm);
//  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
//  vec3 specular = 0.5 * spec * light_color;
//
//  vec3 result = (ambient + diffuse + specular) * color;

  vec4 color = vec4(C_ambient + C_sun + out_lights, 1.0);


  float exposure = 1.0;
  color.rgb = vec3(1.0) - exp(-color.rgb* exposure);

  //@todo check how to get the precise value
  float white_point = 350;
  final_color = vec4(0.0, 0.0, 0.0, 1.0);
  final_color.rgb = linear_to_hdr10(color.rgb, white_point);
  final_color.rgb = ApplyFog(final_color.rgb, in_view_position);
}
