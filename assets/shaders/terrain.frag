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

layout(binding = 3) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(binding = 1) uniform sampler2D tex_sampler[7];
layout(binding = 2) uniform samplerCube env_sampler[];

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

void main()
{
  //@todo do PBR for terrain ?
  vec4 albedo = vec4(0.0);

  albedo += tex_color(TERRAIN_SAND) * in_weights.x;
  albedo += tex_color(TERRAIN_GROUND) * in_weights.y;
  albedo += tex_color(TERRAIN_GRASS) * in_weights.z;
  albedo += tex_color(TERRAIN_SNOW) * in_weights.w;

  vec3 p = in_position;
  vec3 v = normalize(pc.view_position - p);
  vec3 i = normalize(p - pc.view_position);
  vec3 normal = normalize(in_normal.xyz);

  float metallic = 0.9;
  float roughness = 0.9;

  //vec3 irradiance = texture(cube_maps[ENVIRONMENT_MAP_INDEX], normal).rgb;
  vec3 F0 = mix(vec3(0.04), albedo.xyz, metallic);

  vec4 C_diffuse = albedo / PI;
  vec4 C_specular = vec4(1.0);

  //@todo looks good but is it ok?
  float P = 1.0 - roughness;// * (1.0 - roughness);

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

  for (int i = 0; i < NR_POINT_LIGHTS; ++i) {

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

  vec4 color = vec4(C_ambient + C_sun + out_lights, 1.0);

  // float near = sun_light.near_far.x;
  // float far = sun_light.near_far.y;

  // float cascade_blend = clamp((var.depth - near) / (far - near), 0.0, 1.0);
  // float csm_shadow = CalculateInfiniteShadow(sun_light.light_space_matrix, cascade_blend);
  // float csm_factor = max(csm_shadow, 0.1);
  // color.xyz *= csm_factor;
  color.xyz *= PI;
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

  float exposure = 0.3;
  //color.rgb = vec3(1.0) - exp(-color.rgb* exposure);

  //@todo check how to get the precise value
  float white_point = 350;
  final_color = vec4(0.0, 0.0, 0.0, 1.0);
  final_color.rgb = linear_to_hdr10(color.rgb, white_point);
}
