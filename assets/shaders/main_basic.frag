#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 2
#define PI 3.1415926538
#define DIFFUSE_INDEX 0
#define ALPHA_INDEX 1
#define SPECULAR_INDEX 2
#define NORMAL_INDEX 3
#define SHADOW_MAP_INDEX 4
#define GAMMA_TRESHOLD 0.0031308

layout(location = 0) out vec4 final_color;

layout(location = 0) in FRAG_VAR {
  vec4 color;
  vec4 light_space;
  vec3 light_dir;
  vec3 ltan;
  vec3 normal;
  vec3 position;
  vec4 tangent;
  vec3 view_dir;
  vec3 view_position;
  vec3 vtan;
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

vec3 FetchNormal(out vec3 view_dir, out vec3 light_dir)
{
  vec3 normal = var.normal;
  view_dir = var.view_dir;
  light_dir = var.light_dir;

  ivec2 tex_size = textureSize(tex_sampler[NORMAL_INDEX], 0);

  //@todo get rid of if
  if (tex_size.x != 1 && tex_size.y != 1) {
    vec3 m = texture(tex_sampler[NORMAL_INDEX], var.texture_coord).xyz;
    //vec3 n = normalize(var.normal);
    //vec3 t = normalize(var.tangent.xyz - n * dot(var.tangent.xyz, n));
    //vec3 b = cross(var.normal, var.tangent.xyz) * var.tangent.w;
    //normal = normalize(vec3(t * m.x + b * m.y + n * m.z));
    normal = normal * 2.0 - 1.0;
    normal.z = sqrt(1.0 - normal.x * normal.x - normal.y * normal.y);

    view_dir = var.vtan;
    light_dir = var.ltan;
  }

  view_dir = normalize(view_dir);
  light_dir = normalize(light_dir);

  return normalize(normal);
}

float InverseSquareAttenuation(vec3 p, vec3 l)
{
  float r = 5.0;
  float r_max = 50.0;
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
  float r_max = 7.0;
  vec3 atten_const = vec3(1.0);
  atten_const.x = -(k * k) / (r_max * r_max);
  atten_const.y = 1.0 / (1.0 - exp(-(k * k)));
  atten_const.z = exp(-(k * k)) / (1.0 - exp(-(k * k)));

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
  vec4 coord = light_space / light_space.w;
  //coord.y = 1.0 - coord.y;
  ivec2 tex_dim = textureSize(tex_sampler[SHADOW_MAP_INDEX], 0);
  float scale = 1.5;
  float dx = 1.0 / float(tex_dim.x);
  float dy = 1.0 / float(tex_dim.y);

  float shadow = 0.0;
  int count = 0;
  int range = 1;
  //vec3 light_dir = normalize(-sun_light.direction);
  vec3 light_dir = normalize(var.light_dir);
  float bias = max(0.05 * (1.0 - dot(normalize(normal), light_dir)), 0.005);

  for (int x = -range; x <= range; x++)
  {
    for (int y = -range; y <= range; y++)
    {
      float pcf_depth = texture(tex_sampler[SHADOW_MAP_INDEX], coord.xy + vec2(x*dx, y*dy)).x;
      shadow += coord.z - bias > pcf_depth ? 1.0 : 0.0;
      count++;
    }
  
  }
  shadow /= 13;

  if(coord.z > 1.0) {
    shadow = 0.0;
  }

  return (1.0 - shadow);
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
  vec3 light_color = sun_light.color;
  vec3 specular_color = texture(tex_sampler[SPECULAR_INDEX], var.texture_coord).rgb;
  vec3 view_dir; vec3 light_dir;
  vec3 normal = FetchNormal(view_dir, light_dir);
  vec3 halfway_dir = normalize(light_dir + view_dir);
  float nl = dot(normal, halfway_dir);
  float shadow = ShadowCalculation(var.light_space, normal);

  //float luminance = 0.212639 * light_color.r + 0.715169 * light_color.g + 0.072192 * light_color.b;

  //vec3 spot = max(cos(var.position.z / (var.position)), 0);
  float attenuation = InverseSquareAttenuation(var.position, sun_light.position);
  vec3 color = attenuation * DiffuseReflection(normal, light_dir, light_color, ambient_color, diffuse_color.xyz);

  ivec2 tex_size = textureSize(tex_sampler[SPECULAR_INDEX], 0);

  //@todo get rid of if
  if (tex_size.x != 1 && tex_size.y != 1) {
    color += SpecularReflection(normal, halfway_dir, material.shi_ior_diss.x, nl, light_color, specular_color);
  } 

  color *= shadow;

  //color = color / (color + vec3(1.0));
  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);
  //color = pow(color, vec3(1.0 / 2.2));

  final_color = vec4(color, 1.0);
}

