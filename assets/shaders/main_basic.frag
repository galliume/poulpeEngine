#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define TEXTURE_COUNT 8

//env options
#define HAS_FOG 0 //(<< 0)
#define HAS_IRRADIANCE 1

//mesh options
#define HAS_BASE_COLOR 0
#define HAS_SPECULAR 1
#define HAS_NORMAL 2
#define HAS_ALPHA 3
#define HAS_MR 4
#define HAS_EMISSIVE 5
#define HAS_AO 6
#define HAS_TRANSMISSION 7

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
  float tangent_w;
  mat4 sun_light;
  mat4 model;
  vec4 cascade_coord;
  float depth;
  vec3 blend;
  vec3 n;
} var;

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
  Material material;
};

layout(binding = 3) uniform samplerCubeShadow tex_shadow_sampler;

layout(binding = 4) uniform samplerCube env_sampler[];

layout(binding = 5) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(binding = 6) uniform sampler2DArrayShadow csm;

layout(push_constant) uniform constants
{
  mat4 view;
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

float InverseSquareAttenuation(vec3 l)
{
  float r = 20.0;
  float r_max = 100.0;
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
  float shadow_offset = 2.f/2048.f;//@todo push constant
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
  limit.xy -= oxy * (1.f / 2048.f);
  limit.yz -= oyz * (1.f / 2048.f);

  //float depth = depth_transform.x + depth_transform.y / m;
  float depth = length(p) / 500.0f;//far plane
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

float CalculateInfiniteShadow(vec3 cascade_coord0, vec3 cascade_blend)
{
  vec3 p1, p2;
  vec3 cascade_coord1 = cascade_coord0 * sun_light.cascade_scale1 + sun_light.cascade_offset1;
  vec3 cascade_coord2 = cascade_coord0 * sun_light.cascade_scale2 + sun_light.cascade_offset2;
  vec3 cascade_coord3 = cascade_coord0 * sun_light.cascade_scale3 + sun_light.cascade_offset3;

  bool beyond_cascade2 = (cascade_blend.y >= 0.0);
  bool beyond_cascade3 = (cascade_blend.z >= 0.0);
  p1.z = float(beyond_cascade2) * 2.0;
  p2.z = float(beyond_cascade3) * 2.0 + 1.0;

  vec2 shadow_coord1 = (beyond_cascade2) ? cascade_coord2.xy : cascade_coord0.xy;
  vec2 shadow_coord2 = (beyond_cascade3) ? cascade_coord3.xy : cascade_coord1.xy;

  float depth1 = (beyond_cascade2) ? cascade_coord2.z : cascade_coord0.z;
  float depth2 = (beyond_cascade3) ? clamp(cascade_coord3.z, 0.0, 1.0) : cascade_coord1.z;
  depth1 += 0.0001;
  depth2 += 0.0001;

  vec3 blend = clamp(cascade_blend, 0.0, 1.0);
  float weight = (beyond_cascade2) ? blend.y - blend.z : 1.0 - blend.x;

  float texel_size1 = sun_light.cascade_texel_size;
  float delta = 3.0 * texel_size1;

  vec4 shadow_offset[2] = vec4[2](
    vec4(-texel_size1, -delta, delta, -texel_size1),
    vec4(texel_size1, delta, -delta, texel_size1)
  );

  p1.xy = shadow_coord1 + shadow_offset[0].xy;
  float light1 = texture(csm, vec4(p1.xy, p1.z, depth1));
  p1.xy = shadow_coord1 + shadow_offset[0].zw;
  light1 += texture(csm, vec4(p1.xy, p1.z, depth1));
  p1.xy = shadow_coord1 + shadow_offset[1].xy;
  light1 += texture(csm, vec4(p1.xy, p1.z, depth1));
  p1.xy = shadow_coord1 + shadow_offset[1].zw;
  light1 += texture(csm, vec4(p1.xy, p1.z, depth1));

  p2.xy = shadow_coord2 + shadow_offset[0].xy;
  float light2 = texture(csm, vec4(p2.xy, p2.z, depth2));
  p2.xy = shadow_coord2 + shadow_offset[0].zw;
  light2 += texture(csm, vec4(p2.xy, p2.z, depth2));
  p2.xy = shadow_coord2 + shadow_offset[1].xy;
  light2 += texture(csm, vec4(p2.xy, p2.z, depth2));
  p2.xy = shadow_coord2 + shadow_offset[1].zw;
  light2 += texture(csm, vec4(p2.xy, p2.z, depth2));

  return (mix(light2, light1, weight) * 0.25);
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

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
  float P = 5.0;
  vec3 F = F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), P);

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

vec3 ApplyFog(vec3 shaded_color, vec3 v, float height)
{
  float fog_density = 0.09;
  float height_falloff = 0.4;
  vec3 fog_color = vec3(0.4, 0.45, 0.5);

  float f = exp(-fog_density * length(v) * exp(-height_falloff * height));
  return (mix(fog_color, shaded_color, f));
}

void main()
{
  if (bool((pc.options >> HAS_ALPHA) & 1u) && material.alpha.x == 1.0) {
    vec4 alpha_color = texture(tex_sampler[ALPHA_INDEX], var.texture_coord);
    if (alpha_color.r < material.alpha.y) discard;
  }

  vec2 normal_coord = transform_uv(
    material.normal_translation,
    material.normal_scale,
    material.normal_rotation,
    var.texture_coord);

  vec3 N_local = normalize(var.norm);
  vec3 T_local = normalize(var.TBN[0]);
  T_local = normalize(T_local - dot(T_local, N_local) * N_local);
  vec3 B_local = cross(N_local, T_local) * var.tangent_w;
  mat3 pureTBN = mat3(T_local, B_local, N_local);

  vec2 xy = texture(tex_sampler[NORMAL_INDEX], normal_coord).rg * 2.0 - 1.0;
  xy.y = -xy.y;
  float z = sqrt(max(0.0, 1.0 - dot(xy, xy)));
  vec3 texture_normal = pureTBN * vec3(xy, z);

  float has_normal = ((pc.options >> HAS_NORMAL) & 1u);
  vec3 normal = mix(var.norm, texture_normal, has_normal);
  normal = normalize(normal);

  vec2 mr_coord = transform_uv(
    material.mr_translation,
    material.mr_scale,
    material.mr_rotation,
    var.texture_coord);

  float metallic = material.mre_factor.x;
  float roughness = material.mre_factor.y;

  if (bool((pc.options >> HAS_MR) & 1u)) {
    vec3 mr = texture(tex_sampler[METAL_ROUGHNESS_INDEX], mr_coord).rgb;
    metallic = mr.b;
    roughness = mr.g;
  }

  roughness = max(roughness, 0.04);

  // vec2 transmission_coord = transform_uv(
  //   material.transmission_translation,
  //   material.transmission_scale,
  //   material.transmission_rotation,
  //   var.texture_coord);

  // vec2 transmission = texture(tex_sampler[TRANSMISSION_INDEX], transmission_coord).bg;
  // transmission.xy *= material.strength.z;
  // transmission *= float((pc.options >> HAS_TRANSMISSION) & 1u);

  float ao = material.strength.y;
  if (bool((pc.options >> HAS_AO) & 1u))
  {
    float ao_rate = texture(tex_sampler[AO_INDEX], var.texture_coord).r;
    ao = ao_rate;
  }

  vec2 diffuse_coord = transform_uv(
    material.diffuse_translation,
    material.diffuse_scale,
    material.diffuse_rotation,
    var.texture_coord);

  float has_base_color = ((pc.options >> HAS_BASE_COLOR) & 1u);
  vec4 C_diffuse = mix(material.base_color, texture(tex_sampler[DIFFUSE_INDEX], diffuse_coord), has_base_color);
  float color_alpha = C_diffuse.a;

  if (material.alpha.x == 1.0 && color_alpha < material.alpha.y) discard;
  if (material.base_color.a < material.alpha.y) discard;

  vec3 p = var.frag_pos;
  vec3 v = var.view_pos;
  vec3 V = normalize(v);

  vec3 F0 = mix(vec3(0.04), C_diffuse.xyz, metallic);

  // vec4 C_specular = material.specular;
  // if (bool((pc.options >> HAS_SPECULAR) & 1u)) {
  //   C_specular = texture(tex_sampler[SPECULAR_INDEX], var.texture_coord);
  // };

  vec3 out_lights = vec3(0.0);

  vec3 F90 = vec3(1.0);

  //directional sun light
  //vec3 l = normalize(-sun_light.direction);
  float d = length(sun_light.position - p) * length(sun_light.position - p);
  //float d = 100000.0;//sun distance approx ?
  float radius = d * tan(0.00463);//sun approx angle
  vec3 L = normalize(sun_light.position);
  vec3 H = normalize(L + V);
  float NdL = max(dot(normal, L), 0.0);
  float NdV = max(dot(normal, V), 0.0001);
  float NdH = max(dot(normal, H), 0.0);
  float HdV = max(dot(H, V), 0.0);

  //float f = ReflectionBounce(F0);
  vec3 F = FresnelSchlick(HdV, F0);
  float sun_roughness = roughness + (radius / (2 * d));
  float D = GGXDistribution(NdH, sun_roughness);
  float G1 = SmithGeometryGGX(sun_roughness, NdV);
  float G2 = SmithGeometryGGX(sun_roughness, NdL);
  float G = G1 * G2;

  vec3 kD = vec3(1.0) - F;
  kD *= 1.0 - metallic;

  vec3 numerator    = D * G * F;
  float denominator = 4.0 * max(NdV, 0.04) * max(NdL, 0.04);
  vec3 specular     = numerator / denominator;
  specular = clamp(specular, 0.0, 10.0);
  vec3 radiance = sun_light.color.rgb;

  vec3 csm_coords = var.cascade_coord.xyz / var.cascade_coord.w;
  float csm_shadow = CalculateInfiniteShadow(csm_coords, var.blend);
  vec3 C_sun = (kD * (C_diffuse.rgb / PI) + specular) * radiance * NdL * csm_shadow * 0.01;

  vec3 kS_amb = FresnelSchlickRoughness(max(dot(normal, V), 0.0), F0, roughness);
  vec3 kD_amb = (1.0 - kS_amb) * (1.0 - metallic);

  vec3 diff_irradiance = vec3(0.0);
  vec3 spec_irradiance = vec3(0.0);

  if (bool((pc.env_options >> HAS_IRRADIANCE) & 1u))
  {
    float lod = 9.0;
    vec3 r = reflect(-V, normal);
    diff_irradiance = textureLod(env_sampler[ENVIRONMENT_MAP_INDEX], normal, lod).rgb;
    spec_irradiance = textureLod(env_sampler[ENVIRONMENT_MAP_INDEX], r, roughness * lod).rgb;
    //irradiance = srgb_to_linear(irradiance);
  }
  
  vec3 C_ambient = ((kD_amb * C_diffuse.rgb * diff_irradiance) + (kS_amb * spec_irradiance)) * ao * NdV * 0.01;

  for (int i = 0; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = point_lights[i].position;
    vec3 l = light_pos - p;

    //@todo check thoses attenuation functions
    //float d = length(light_pos - p);
    float attenuation = InverseSquareAttenuation(l);
    //float attenuation = 1.0 / (point_lights[i].clq.x + point_lights[i].clq.y * d + point_lights[i].clq.z * (d * d));
    //vec3 C_light = srgb_to_linear(point_lights[i].color.rgb) * attenuation;
    vec3 C_light = vec3(1.0f) * attenuation;

    vec3 L = normalize(normalize(l));
    vec3 H = normalize(V + L);
    float NdL = max(dot(normal, L), 0.0);
    float NdV = max(dot(normal, V), 0.0001);
    float NdH = max(dot(normal, H), 0.0);
    float HdV = max(dot(H, V), 0.0);

    vec3 F  = FresnelSchlick(HdV, F0);
    float D = GGXDistribution(NdH, roughness);
    float G = SmithGeometryGGX(roughness, NdV) * SmithGeometryGGX(roughness, NdL);

    vec3 numerator    = D * G * F;
    float denominator = 4.0 * max(NdV, 0.04) * max(NdL, 0.04);
    vec3 specular     = numerator / denominator;
    specular = clamp(specular, 0.0, 10.0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 curr = (kD * (C_diffuse.rgb) + specular) * C_light * NdL;

    if (i == 1) {
      vec3 frag_to_light = p - point_lights[i].position;
      float shadow = ShadowCalculation(frag_to_light, point_lights[i], NdL);
      curr *= max(shadow, 0.1);
    }
    out_lights += curr;
  }

  // C_sun *= 0.05;
  // C_ambient *= 0.05;
  vec4 color = vec4(C_ambient + C_sun + out_lights, color_alpha);
  //color.xyz *= PI;
  
  vec2 emissive_coord = transform_uv(
    material.emissive_translation,
    material.emissive_scale,
    material.emissive_rotation,
    var.texture_coord);

  vec4 emissive_color = texture(tex_sampler[EMISSIVE_INDEX], emissive_coord);

  vec3 emissive = ((pc.options >> HAS_EMISSIVE) & 1u) * emissive_color.xyz;
  emissive *= material.mre_factor.z * 10;
  color.rgb += emissive;

  float has_fog = ((pc.env_options >> HAS_FOG) & 1u);
  color.rgb = mix(color.rgb, ApplyFog(color.rgb, v, p.y), has_fog);
//
//  color.r = linear_to_sRGB(color.r);
//  color.g = linear_to_sRGB(color.g);
//  color.b = linear_to_sRGB(color.b);

  //final_color = color;
  //@todo check how to get the precise value
  float white_point = 350;
  final_color = vec4(0.0, 0.0, 0.0, color_alpha);
  final_color.rgb = linear_to_hdr10(color.rgb, white_point);
  if (material.alpha.x == 2.0) {
    final_color = vec4(linear_to_hdr10(color.rgb, white_point), color_alpha);
  }
  //float exposure = 1.0;
  //final_color.rgb = vec3(1.0) - exp(-final_color.rgb * exposure);
  //final_color = vec4(normal * 0.5 + 0.5, 1.0);
//final_color *= var.env_options;
  //final_color.rgb = vec3(csm_coords.x, csm_coords.y, 0.0f);
}