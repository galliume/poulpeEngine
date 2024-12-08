#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 2
#define DIFFUSE_INDEX 0
#define ALPHA_INDEX 1
#define SPECULAR_INDEX 2
#define NORMAL_INDEX 3

#define SHADOW_MAP_INDEX 0

#define PI 3.1415926538
#define GAMMA_TRESHOLD 0.0031308

layout(location = 0) out vec4 final_color;

layout(location = 0) in FRAG_VAR {
  vec4 color;
  vec4 light_space;
  vec4 tangent;
  vec3 light_dir;
  vec3 normal;
  vec3 position;
  vec3 view_dir;
  vec3 view_position;
  vec2 texture_coord;
  vec3 light_pos_model_space;
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

layout(binding = 1) uniform sampler2D tex_sampler[4];

layout(binding = 2) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

layout(binding = 3) uniform sampler2DShadow tex_shadow_sampler[1];

vec3 DiffuseReflection(vec3 n, vec3 l, vec3 light_color, vec3 ambient_color, vec3 diffuse_color)
{
  vec3 direct_color = light_color * clamp(dot(n, l), 0.0, 1.0);
  return (ambient_color + direct_color) * diffuse_color;
}

vec3 SpecularReflection(vec3 n, vec3 l, float alpha, float nl, vec3 light_color, vec3 specular_color)
{
  float highlight = pow(clamp(dot(n, l), 0.0, 1.0), alpha) * float(nl > 0.0);
  return (light_color * specular_color * highlight);
}

float InverseSquareAttenuation(vec3 p, vec3 l)
{
  float r = 5.0;
  float r_max = 0.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = (r * r) / (r_max * r_max - r * r);
  atten_const.y = (r_max * r_max);

  vec3 l_dir = l - p;
  float r2 = dot(l_dir, l_dir);
  return clamp(atten_const.x * (atten_const.y / r2 - 1.0), 0.0, 1.0);
}

float ExponentialAttenuation(vec3 p, vec3 l)
{
  float k = 2.0;
  float r_max = 20.0;
  vec3 atten_const = vec3(1.0);
  float neg_k_square = -(k * k);
  float exp_k_neg_square = exp(neg_k_square);
  atten_const.x = neg_k_square / (r_max * r_max);
  atten_const.y = 1.0 / (1.0 - exp_k_neg_square);
  atten_const.z = exp_k_neg_square / (1.0 - exp_k_neg_square);

  vec3 l_dir = l - p;
  float r2 = dot(l_dir, l_dir);
  return clamp(exp(r2 * atten_const.x) * atten_const.y - atten_const.z, 0.0, 1.0);
}

float SmoothAttenuation(vec3 p, vec3 l)
{
  float r_max = 15.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

  vec3 l_dir = l - p;
  float r2 = dot(l_dir, l_dir);
  return clamp(r2 * atten_const.x * (sqrt(r2) * atten_const.y - 3.0) + 1.0, 0.0, 1.0);
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
  
//  p.x -= dx;
//  p.y -= dy;
//  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
//  p.x += dx * 2.0;
//  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
//  p.y += dy * 2.0;
//  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);
//  p.x += dx * 2.0;
//  light += texture(tex_shadow_sampler[SHADOW_MAP_INDEX], p, bias);

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
  light *= 0.2;

  return light;
}

void main()
{
  vec4 alpha_color = texture(tex_sampler[ALPHA_INDEX], var.texture_coord);
  ivec2 alpha_size = textureSize(tex_sampler[ALPHA_INDEX], 0);

  if (alpha_size.x != 1 && alpha_size.y != 1 && alpha_color.r < 0.2) {
    discard;
  }

  vec4 diffuse = texture(tex_sampler[DIFFUSE_INDEX], var.texture_coord);
  vec4 diffuse_color = (vec4(material.diffuse, 1.0) / PI) * diffuse;
  if (diffuse_color.a < 0.2) discard;

  vec3 ambient_color = diffuse.xyz * PI;
  vec3 sun_light_color = sun_light.color;
  vec3 specular_color = texture(tex_sampler[SPECULAR_INDEX], var.texture_coord).rgb;
  vec3 view_dir = normalize(var.view_dir);
  vec3 light_dir = normalize(var.light_dir);
  vec3 normal = normalize(var.normal);
  vec3 halfway_dir = normalize(light_dir + view_dir);
  float nl = dot(normal, halfway_dir);
  float shadow = ShadowCalculation(var.light_space, normal);

  //vec3 spot = max(cos(var.position.z / (var.position)), 0);
  
  //sun as a spot light
  //vec3 color = diffuse_color.xyz;
  vec3 p = var.position;
  vec3 l = sun_light.position;
  float attenuation = ExponentialAttenuation(p, l);
  vec3 color = DiffuseReflection(normal, light_dir, sun_light_color, ambient_color, diffuse_color.xyz);
  
  //color *= shadow;
  //color *= attenuation;
  ivec2 tex_size = textureSize(tex_sampler[SPECULAR_INDEX], 0);

  //@todo get rid of if
  if (tex_size.x != 1 && tex_size.y != 1) {
    color += SpecularReflection(normal, halfway_dir, material.shi_ior_diss.x, nl, sun_light_color, specular_color);
  }

  //color *= attenuation;

  //color = color / (color + vec3(1.0));
  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);
  //color = pow(color, vec3(1.0 / 2.2));

  final_color = vec4(color, 1.0);
}

