#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 2

layout(location = 0) out vec4 final_color;

layout(location = 0) in FRAG_VAR {
  mat3 TBN;
  vec4 sun_light_space;
  vec4 view_position;
  vec4 color;
  vec3 normal;
  vec3 position;
  vec3 tangent_light_pos;
  vec3 tangent_frag_pos;
  vec3 tangent_view_pos;
  vec2 texture_coord;
  flat int texture_ID;
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

//0: main texture
//1: alpha
//2: specular map
//3: bump map
//4: shadow map
layout(binding = 1) uniform sampler2D tex_sampler[5];

layout(binding = 2) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
  Material material;
};

vec3 CalcDirLight(vec4 color, vec3 ambient, Light light, vec3 light_dir, vec3 normal, vec3 view_dir, float shadow);
vec3 CalcPointLight(vec4 color, vec3 ambient, Light light, vec3 normal, vec3 view_dir, float shadow);

float ShadowCalculation(vec4 lightSpace, vec3 normal);

void main()
{
  vec3 normal = normalize(var.normal);
  float shadow_ambient = ShadowCalculation(var.sun_light_space, normal);
  
  ivec2 tex_size = textureSize(tex_sampler[3], 0);

  if (tex_size.x != 1 && tex_size.y != 1) {
    normal = texture(tex_sampler[3], var.texture_coord).xyz;
    normal = normalize(normal * 2.0 - 1.0);
  }

  vec4 texture_color = texture(tex_sampler[0], var.texture_coord);
  vec3 ambient_color = vec3(1.0) * 0.3;

  vec3 view_dir = normalize(var.tangent_view_pos.xyz - var.tangent_frag_pos.xyz);
  vec3 sun_light_dir = normalize(-sun_light.direction);

  vec3 color = CalcDirLight(texture_color, ambient_color, sun_light, sun_light_dir, normal, view_dir, shadow_ambient);
//
//  for(int i = 0; i < NR_POINT_LIGHTS; i++) {
//    color += CalcPointLight(texture_color, ambient_color, point_lights[i], normal, view_dir, shadow_ambient);
//  }

  ivec2 mask_size = textureSize(tex_sampler[1], 0);
  if (mask_size.x != 1 && mask_size.y != 1) {
    vec4 mask = texture(tex_sampler[1], var.texture_coord);
//        fColor = mix(fColor, mask, mask.a);
    if (mask.r < 0.2) discard;
  }
  if (texture_color.a < 0.1) discard;


  //color = color / (color + vec3(1.0));
  //color = vec3(-shadowAmbient);
  //color = pow(color, vec3(1.0 / 2.2));

  final_color = vec4(color, 1.0);
}

vec3 CalcDirLight(vec4 color, vec3 ambient, Light light, vec3 light_dir, vec3 normal, vec3 view_dir, float shadow)
{
  float diff = max(dot(normal, light_dir), 0.0);
  vec3 diffuse =  (diff * vec3(texture(tex_sampler[0], var.texture_coord)));

  //vec3 reflectDir = reflect(-light.position, normal);
  vec3 halfway_dir = normalize(light_dir + view_dir);
  float spec = pow(max(dot(normal, halfway_dir), 0.0), 1);

  vec3 specular = vec3(1.f);
  ivec2 tex_size = textureSize(tex_sampler[2], 0);

  if (tex_size.x == 1 && tex_size.y == 1) {
    specular = (material.specular * spec) * light.color;
  } else {
    specular = (vec3(texture(tex_sampler[2], var.texture_coord)) * spec) * light.color;
  }

  return (ambient + diffuse + specular) * color.xyz;
}

vec3 CalcPointLight(vec4 color, vec3 ambient, Light light, vec3 normal, vec3 view_dir, float shadow)
{
  vec3 light_dir = normalize(-light.direction);

  float distance = length(light.position - var.position);
  float attenuation = 1.0 / (light.clq.x + light.clq.y * distance + light.clq.z * (distance * distance));

  float diff = max(dot(normal, light_dir), 0.0);
  vec3 diffuse =  (diff * vec3(texture(tex_sampler[0], var.texture_coord)));

  //vec3 reflectDir = reflect(-light.position, normal);
  vec3 halfway_dir = normalize(light_dir + view_dir);
  float spec = pow(max(dot(normal, halfway_dir), 0.0), 1);

  vec3 specular = vec3(1.f);
  ivec2 tex_size = textureSize(tex_sampler[2], 0);

  if (tex_size.x == 1 && tex_size.y == 1) {
    specular = (material.specular * spec);
  } else {
    specular = (vec3(texture(tex_sampler[2], var.texture_coord)) * spec) ;
  }
  ambient *= light.color * light.ads.x;
  diffuse *= light.color * light.ads.y;
  specular *= light.color * light.ads.z;

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return (ambient + diffuse + specular) * color.xyz;
}

float ShadowCalculation(vec4 light_space, vec3 normal)
{
  vec4 coord = light_space / light_space.w;
  //coord.y = 1.0 - coord.y;
  ivec2 tex_dim = textureSize(tex_sampler[4], 0);
  float scale = 1.5;
  float dx = 1.0 / float(tex_dim.x);
  float dy = 1.0 / float(tex_dim.y);

  float shadow = 0.0;
  int count = 0;
  int range = 1;
  //vec3 light_dir = normalize(-sun_light.direction);
  vec3 light_dir = normalize(sun_light.position - var.position);
  float bias = max(0.05 * (1.0 - dot(normalize(normal), light_dir)), 0.005);

  for (int x = -range; x <= range; x++)
  {
    for (int y = -range; y <= range; y++)
    {
      float pcf_depth = texture(tex_sampler[4], coord.xy + vec2(x*dx, y*dy)).x;
      shadow += coord.z - bias > pcf_depth ? 1.0 : 0.0;
      count++;
    }
  
  }
  shadow /= count;

  if(coord.z > 1.0) {
    shadow = 0.0;
  }

  return (1.0 - shadow);
}