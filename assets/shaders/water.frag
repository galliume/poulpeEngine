#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define GAMMA_TRESHOLD 0.0031308
#define PI 3.141592653589793238462643383279
#define NR_POINT_LIGHTS 2

#define DIFFUSE_INDEX 0
#define DEPTH_INDEX 1
#define NORMAL_INDEX 2
#define NORMAL2_INDEX 3
#define ENV_INDEX 5

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

layout(location = 0) out vec4 final_color;

layout(location = 0) in vec2 in_texture_coord;
layout(location = 1) in vec3 in_position;
layout(location = 2) in vec3 in_view_position;
layout(location = 3) in mat3 in_TBN;
layout(location = 6) in vec3 in_normal;

// CSM inputs from tessellation shader
layout(location = 7) in float in_depth;
layout(location = 8) in vec4 in_cascade_coord;
layout(location = 9) in vec4 in_cascade_coord1;
layout(location = 10) in vec4 in_cascade_coord2;
layout(location = 11) in vec4 in_cascade_coord3;
layout(location = 12) in vec3 in_cascade_blend;

layout(binding = 1) uniform sampler2D tex_sampler[5];
layout(binding = 2) uniform samplerCube env_sampler[];
layout(binding = 3) readonly buffer LightObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(binding = 4) uniform sampler2DArrayShadow csm;

layout(push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
  vec4 options;
} pc;

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

vec3 tex_color(int index)
{
  vec3 color = texture(tex_sampler[index], in_texture_coord).rgb;
  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);

  return color;
}

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0;
  float near = 0.1;
  float far = 1000.0;

  return (2.0 * near * far) / (far + near - z * (far - near));
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
  float a = 1.0 - pow(1.0 - max(NdL, 0.001), 5);
  float b = 1.0 - pow(1.0 - max(NdV, 0.001), 5);

  return (21.0 / (20.0 * PI)) * (1.0 - F0) * diffuse * a * b;
}

//https://blog.selfshadow.com/publications/blending-in-detail/
vec3 blend_rnm(vec3 n1, vec3 n2)
{
  vec3 t = n1.xyz * vec3( 2,  2, 2) + vec3(-1, -1,  0);
  vec3 u = n2.xyz * vec3(-2, -2, 2) + vec3( 1,  1, -1);
  vec3 r = t * dot(t, u) - u * t.z;
  return normalize(r);
}

float SmoothAttenuation(vec3 l, float r_max)
{
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

  float r2 = dot(l, l);
  return max(r2 * atten_const.x * (sqrt(r2) * atten_const.y - 3.0) + 1.0, 0.0001);
}

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

  float delta = 3.0f / 16.0f * (1.0f / 2048.f);
  vec4 shadow_offset[2] = vec4[2](
    vec4(-delta, -3.0 * delta, 3.0 * delta, -delta),
    vec4(delta, 3.0 * delta, -3.0 * delta, delta)
  );

  float max_bias = 0.005;
  float min_bias = 0.0005;
  float bias = 0.0;//0.001;max(max_bias * (1.0 - NdL), min_bias);

  float light1 = 0.0;
  light1 += texture(csm, vec4(shadow_coord1.xy + shadow_offset[0].xy, cascade_index, shadow_coord1.z - bias));
  light1 += texture(csm, vec4(shadow_coord1.xy + shadow_offset[0].zw, cascade_index, shadow_coord1.z - bias));
  light1 += texture(csm, vec4(shadow_coord1.xy + shadow_offset[1].xy, cascade_index, shadow_coord1.z - bias));
  light1 += texture(csm, vec4(shadow_coord1.xy + shadow_offset[1].zw, cascade_index, shadow_coord1.z - bias));

  float light2 = light1;
  if (cascade_index < 3.0) {
    light2 = 0.0;
    light2 += texture(csm, vec4(shadow_coord2.xy + shadow_offset[0].xy, cascade_index + 1.0, shadow_coord2.z - bias));
    light2 += texture(csm, vec4(shadow_coord2.xy + shadow_offset[0].zw, cascade_index + 1.0, shadow_coord2.z - bias));
    light2 += texture(csm, vec4(shadow_coord2.xy + shadow_offset[1].xy, cascade_index + 1.0, shadow_coord2.z - bias));
    light2 += texture(csm, vec4(shadow_coord2.xy + shadow_offset[1].zw, cascade_index + 1.0, shadow_coord2.z - bias));
  }

  return mix(light1, light2, weight) * 0.25;
}

void main()
{
  vec3 deep_color = vec3(3.0 / 255.0, 84.0 / 255.0, 139.0 / 255.0);
  vec3 shallow_color = vec3(64.0 / 255.0, 164.0 / 255.0, 223.0 / 255.0);
  vec4 foam_color = vec4(1.0, 1.0, 1.0, 1.0);

  ivec2 tex_size = textureSize(tex_sampler[DEPTH_INDEX], 0);
  vec2 depth_coord = vec2(gl_FragCoord.x, gl_FragCoord.y);
  depth_coord.x /= tex_size.x;
  depth_coord.y /= tex_size.y;

  float t = pc.options.x;
  vec2 foam_coord = in_texture_coord;
  vec2 scaledUVFoam = foam_coord * 1000;
  vec2 scaledUV = foam_coord * 5000;
  float scroll = pc.options.x * 0.05;

  float channelA = texture(tex_sampler[DIFFUSE_INDEX], scaledUVFoam - vec2(scroll, cos(foam_coord.x))).r;
  float channelB = texture(tex_sampler[DIFFUSE_INDEX], scaledUVFoam * 0.5 + vec2(sin(foam_coord.y), scroll)).b;

  float mask = (channelA + channelB) * 0.95;
  mask = pow(mask, 2);
  mask = clamp(mask, 0, 1);

  float depth_diff = abs(LinearizeDepth(gl_FragCoord.z) - LinearizeDepth(texture(tex_sampler[DEPTH_INDEX], depth_coord).r));
  float falloff_distance = 1.0;
  float edge_falloff = 0.001;
  float bias = 0.002;

  float depth_factor = smoothstep(0.0, 10.0, depth_diff);
  //float foam_factor = smoothstep(0.0, 0.5, depth_diff);

  vec4 color = vec4(mix(shallow_color, deep_color, depth_factor), 1.0);

  if(depth_diff < falloff_distance * edge_falloff)
  {
    float leading = depth_diff / (falloff_distance * edge_falloff);
    color.a *= leading;
    mask *= leading;
  }
  // Calculate linear falloff value
  float fall_off = 1.0 - (depth_diff / falloff_distance) + bias;

  vec3 edge = foam_color.rgb * fall_off * foam_color.a;

//  //@todo do PBR for water ?
  vec2 xy = texture(tex_sampler[NORMAL_INDEX], scaledUV - vec2(scroll, cos(foam_coord.x))).xy;
  xy = xy.xy * 2.0 - 1.0;
  float z = sqrt(1 - dot(xy, xy));
  vec3 normal1 = vec3(xy, z);
  normal1 = normalize(in_TBN * normal1);

  vec2 xy2 = texture(tex_sampler[NORMAL2_INDEX], scaledUV + vec2(sin(foam_coord.y), scroll)).xy;
  xy2 = xy2.xy * 2.0 - 1.0;
  float z2 = sqrt(1 - dot(xy2, xy2));
  vec3 normal2 = vec3(xy2, z2);
  normal2 = normalize(in_TBN * normal2);

  vec3 normal = in_normal;//blend_rnm(normal1, normal2);

  //@todo a point lights...
  //sun directionnal light
  vec3 light_color = sun_light.color.rgb;

  vec3 p = in_position;
  vec3 v = normalize(pc.view_position - p);
  vec3 i = normalize(p - pc.view_position);

  vec3 l = normalize(-sun_light.direction);
  vec3 h = normalize(v + l);

  float HdV = max(dot(h, v), 0.001);
  float NdH = max(dot(normal, h), 0.001);
  float NdL = max(dot(normal, l), 0.001);
  float NdV = max(dot(normal, v), 0.001);

  vec3 F0 = vec3(0.02);
  vec3 F90 = vec3(1.0);
  float metallic = 0.0; // Water is a dielectric
  float P = 1.0;
  float roughness = 0.1; // Lower roughness for sharper reflections

  float D = GGXDistribution(NdH, roughness);
  float G1 = SmithGeometryGGX(roughness, NdV);
  float G2 = SmithGeometryGGX(roughness, NdL);
  float G = G1 * G2;

  float f = ReflectionBounce(F0);
  vec3 F = FresnelSchlick(F0, F90, HdV, P);

  vec3 specular = (D * G * F) / (4.0 * NdV * NdL + 0.0001); // Specular BRDF
  vec3 diffuse = color.rgb; // Diffuse is the refracted color from below

  vec3 kD = vec3(1.0) - F;
  kD *= 1.0 - metallic;

  vec3 absorption_coeff = vec3(0.1);
  vec3 scaterring_coeff = vec3(0.003);
  vec3 extinct_coeff = absorption_coeff + scaterring_coeff;
  vec3 transmittance = exp(-depth_diff * extinct_coeff);


  vec3 r = reflect(i, normalize(in_normal));
  vec3 env_color = vec3(texture(env_sampler[0], r).rgb);

  // Final color is a mix of reflected sky and refracted scene color, controlled by Fresnel
  vec3 surface_color = mix(diffuse, env_color, F);

  vec3 C_sun = (surface_color + specular) * light_color * NdL;

  vec3 csm_coords = in_cascade_coord.xyz / in_cascade_coord.w;
  float csm_shadow = CalculateInfiniteShadow(csm_coords, in_cascade_blend, NdL);
  C_sun *= max(csm_shadow, 0.1); 

  vec3 out_lights = vec3(0.0);

  for (int i = 0; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = point_lights[i].position;
    vec3 l = normalize(light_pos - p);

    //@todo check thoses attenuation functions
    float d = length(light_pos - p);
    float attenuation = 1.0 / (d * d);
    vec3 C_light = point_lights[i].color.rgb * attenuation;

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

    // For point lights, we just add their contribution. The diffuse color is already determined.
    vec3 diff = surface_color.xyz;

    out_lights += (diff + specular) * C_light * NdL;
  }

  //ray marching for transmittance and scaterring
  int max_step = 10;
  int step_offset = 10;
  vec3 ray_pos = in_view_position;
  vec2 coord = depth_coord;

//  vec3 reflect_vector = normalize(reflect(-in_view_position, normal));

//  for (int i = 0; i < max_step; ++i) {
//    ray_pos += reflect_vector * step_offset;
//    coord.x += step_offset;
//    coord.y += step_offset;
//
//    float depth = abs(LinearizeDepth(gl_FragCoord.z) - LinearizeDepth(texture(tex_sampler[DEPTH_INDEX], coord).r));
//    transmittance += exp(-depth * extinct_coeff);
//  }


//  float LdH = max(dot(l, h), 0.0);
//  float reflectance = 0.55f;
//  float f = ReflectionBounce(F0);
//  vec3 F = FresnelSchlick(F0, F90, HdV, P) * f;
//  float D = GGXDistribution(NdH, roughness);
//  float G1 = SmithGeometryGGX(roughness, NdV);
//  float G2 = SmithGeometryGGX(roughness, NdL);
//  float G = G1 * G2;
//
//
//  vec3 specular = (D * G * F) / (4.0 * NdV * NdL + 0.0001);
//  vec3 diffuse = color.rgb / PI;
//  vec3 radiance =  light_color * 3.0;
//
//  vec3 C_sun = (kD * diffuse + specular) * radiance * NdL;
  C_sun.rgb += clamp(edge - vec3(mask), 0.0, color.a);
  vec3 C_ambient = vec3(0.03) * deep_color;
  vec3 result = C_ambient + C_sun + out_lights;

  float exposure = 1.0;
  //result.rgb = vec3(1.0) - exp(-result.rgb* exposure);

  //@todo check how to get the precise value
  float white_point = 350;
  final_color = vec4(0.0, 0.0, 0.0, color.a);
  final_color = vec4(linear_to_hdr10(result.rgb, white_point), color.a);
  //final_color.rgb = ApplyFog(final_color.rgb, in_view_position);

  //final_color = vec4(specular, 1.0);
  //final_color = vec4(normal, 1.0);
  //final_color = vec4(vec3(LinearizeDepth(gl_FragCoord.z)), 1.0);
  //final_color = vec4(vec3(depth_diff, 0.0, 1.0 - depth_diff), 1.0);

  //final_color = vec4(vec3(LinearizeDepth(texture(tex_sampler[DEPTH_INDEX], depth_coord).r))/1000.0, 1.0);
  //result.r = linear_to_sRGB(result.r);
  //result.g = linear_to_sRGB(result.g);
  //result.b = linear_to_sRGB(result.b);

//@todo do PBR for water ?
//  ivec2 tex_size = textureSize(tex_sampler[DEPTH_INDEX], 0);
//  vec2 depth_coord = vec2(gl_FragCoord.x, gl_FragCoord.y);
//  depth_coord.x /= tex_size.x;
//  depth_coord.y /= tex_size.y;
//
//  float t = pc.options.x;
//  vec2 scaledUVForm = in_texture_coord * 1000;
//  vec2 scaledUV = in_texture_coord * 500;
//  float edgePatternScroll = 0.1;
//  float scroll = pc.options.x * 0.2;
//
//  float channelA = texture(tex_sampler[DIFFUSE_INDEX], scaledUVForm - vec2(edgePatternScroll, cos(in_texture_coord.x))).r;
//  float channelB = texture(tex_sampler[DIFFUSE_INDEX], scaledUVForm * 0.5 + vec2(sin(in_texture_coord.y), edgePatternScroll)).b;
//
//  float mask = (channelA + channelB) * 0.95;
//  mask = pow(mask, 2);
//  mask = clamp(mask, 0, 1);
//
//  float depth_diff = abs(LinearizeDepth(gl_FragCoord.z) - LinearizeDepth(texture(tex_sampler[DEPTH_INDEX], depth_coord).r));
//  float falloff_distance = 1.0;
//  float edge_falloff = 0.0;
//  float bias = 0.1;
//
//  vec3 deep_color = vec3(3.0 / 255.0, 84.0 / 255.0, 139.0 / 255.0);
//  vec3 shallow_color = vec3(64.0 / 255.0, 164.0 / 255.0, 223.0 / 255.0);
//  vec4 foam_color = vec4(1.0, 1.0, 1.0, 1.0);
//  float depth_factor = smoothstep(0.0, 10.0, depth_diff);
//  //float foam_factor = smoothstep(0.0, 0.5, depth_diff);
//
//  vec4 color = vec4(mix(shallow_color, deep_color, 1.0), 0.7);
//
//  if(depth_diff < falloff_distance * edge_falloff)
//  {
//    float leading = depth_diff / (falloff_distance * edge_falloff);
//    color.a *= leading;
//    mask *= leading;
//  }
//
//  // Calculate linear falloff value
//  float fall_off = 1.0 - (depth_diff / falloff_distance) + bias;
//  vec3 edge = foam_color.rgb * fall_off * foam_color.a;
//  color.rgb += clamp(edge - vec3(mask), 0.0, 1.0);
//
//  vec3 light_color = vec3(1.0);
//  vec3 ambient = 0.1 * light_color;
//
//  //@todo a point lights...
//
//  //sun directionnal light
//  vec3 light_pos = vec3(0.0, 1000000.0, 0.0);
//
////vec3 norm = normalize(in_normal);
////  vec3 x = dFdx(in_position);
////  vec3 y = dFdy(in_position);
////  vec3 norm = in_inverse_model * normalize(cross(x, y));
//  vec2 xy = texture(tex_sampler[NORMAL_INDEX], scaledUV - vec2(scroll, cos(in_texture_coord.x))).xy;
//  xy = xy.xy * 2.0 - 1.0;
//  float z = sqrt(1 - dot(xy, xy));
//  vec3 normal1 = vec3(xy, z);
//  normal1 = normalize(in_TBN * normal1);
//
//  vec2 xy2 = texture(tex_sampler[NORMAL2_INDEX], scaledUV + vec2(sin(in_texture_coord.y), scroll)).xy;
//  xy2 = xy2.xy * 2.0 - 1.0;
//  float z2 = sqrt(1 - dot(xy2, xy2));
//  vec3 normal2 = vec3(xy2, z2);
//  normal2 = normalize(in_TBN * normal2);
//
//  vec3 normal = normal1;//normal1;//blend_rnm(normal1, normal2);
//
//  vec3 light_dir = normalize(light_pos - in_position);
//  float diff = max(dot(normal, light_dir), 0.0);
//  vec3 diffuse = diff * light_color;
//
//  vec3 view_dir = normalize(pc.view_position - in_position);
//  vec3 reflect_dir = reflect(light_dir, normal);
//  vec3 H = normalize(light_dir + view_dir);
//  float spec = pow(max(dot(normal, H), 0.0), 2.0);
//  vec3 specular = spec * light_color;
//
//  vec3 result = (ambient + diffuse + specular) * color.xyz;
//
//  float exposure = 1.0;
//  result.rgb = vec3(1.0) - exp(-result.rgb* exposure);
//
//  //@todo check how to get the precise value
//  float white_point = 350;
//  final_color = vec4(0.0, 0.0, 0.0, color.a);
//  final_color = vec4(linear_to_hdr10(result.rgb, white_point), color.a);
}
