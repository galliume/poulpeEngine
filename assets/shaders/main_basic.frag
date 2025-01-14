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
#define BASE_COLOR_INDEX 7

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
  vec4 color;
  vec3 tangent;
  vec3 bitangent;  
  mat3 TBN;
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
  vec3 normal_translation;//z: 0 no translation 1.0 translation
  vec3 normal_scale; //z: 0 no scale 1.0 scale
  vec3 normal_rotation; //y: 0 no rotation 1.0 rotation
  vec3 diffuse_translation;//z: 0 no translation 1.0 translation
  vec3 diffuse_scale; //z: 0 no scale 1.0 scale
  vec3 diffuse_rotation; //y: 0 no rotation 1.0 rotation
  vec3 emissive_translation;//z: 0 no translation 1.0 translation
  vec3 emissive_scale; //z: 0 no scale 1.0 scale
  vec3 emissive_rotation; //y: 0 no rotation 1.0 rotation
  vec3 strength;//x: normal strength, y occlusion strength
};

layout(binding = 1) uniform sampler2D tex_sampler[TEXTURE_COUNT];

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

float SmoothAttenuation(vec3 l_dir)
{
  float r_max = 5.0;
  vec2 atten_const = vec2(1.0);
  atten_const.x = 1.0 / (r_max * r_max);
  atten_const.y = 2.0 / r_max;

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

//https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md
vec2 transform_uv(vec3 t, vec3 s, vec3 r, vec2 c)
{
  mat3 translation = mat3(1,0,0, 0,1,0, t.x, -t.y, 1);
  mat3 rotation = mat3(
      cos(r.x), sin(r.x), 0,
     -sin(r.x), cos(r.x), 0,
      0, 0, 1);
  mat3 scale = mat3(s.x, 0, 0, 0, s.y, 0, 0, 0, 1);
  mat3 matrix = translation * rotation * scale;
  vec2 uvTransformed = (matrix * vec3(c.xy, 1)).xy;

  return uvTransformed;
}

void main()
{
  vec4 alpha_color = texture(tex_sampler[ALPHA_INDEX], var.texture_coord);
  ivec2 alpha_size = textureSize(tex_sampler[ALPHA_INDEX], 0);

  if (alpha_size.x != 1 && alpha_size.y != 1) {
    if (material.alpha.x == 1.0 && alpha_color.r < material.alpha.y) discard;
  }
 
//  vec2 normal_coord = transform_uv(
//    material.normal_translation,
//    material.normal_scale,
//    material.normal_rotation,
//    var.texture_coord); 

  vec3 normal = vec3(1.0); 
  normal.xy = texture(tex_sampler[NORMAL_INDEX], var.texture_coord).xy;
  normal.xy = normal.xy * 2.0 - 1.0;
  normal.z = sqrt(1 - dot(normal.xy, normal.xy));

  ivec2 normal_size = textureSize(tex_sampler[NORMAL_INDEX], 0);
  if (normal_size.x <= 2.0) {
    normal = var.norm;
    normal.z = sqrt(1 - dot(normal.xy, normal.xy));
    normal = var.TBN * normal;
  }

  normal = normalize(normal * material.strength.x);

  vec2 metal_roughness = texture(tex_sampler[METAL_ROUGHNESS_INDEX], var.texture_coord).rg;
  float metallic = metal_roughness.r;
  float roughness = metal_roughness.g;

  ivec2 mr_size = textureSize(tex_sampler[METAL_ROUGHNESS_INDEX], 0);
  if (mr_size.x == 1 && mr_size.y == 1) {
    metallic = material.mr_factor.x;
    roughness = material.mr_factor.y;
  }
  metallic = max(metallic, 0.1);
  roughness = max(roughness, 0.1);
  
  float roughness2 = roughness * roughness; 
  roughness2 = roughness2 * roughness2;

  float ao = texture(tex_sampler[AO_INDEX], var.texture_coord).r;
  ivec2 ao_size = textureSize(tex_sampler[AO_INDEX], 0);
  if (ao_size.x == 1 && ao_size.y == 1) {
    ao = 1.0;
  }
  //ao *= material.strength.y;//occlusion strength

//  vec2 diffuse_coord = var.texture_coord;
//  if (material.diffuse_translation.z >1.5) {
//    diffuse_coord = transform_uv(
//      material.diffuse_translation,
//      material.diffuse_scale,
//      material.diffuse_rotation,
//      var.texture_coord);
//  }

  vec4 albedo = texture(tex_sampler[DIFFUSE_INDEX], var.texture_coord);
  float color_alpha = albedo.a;
  
  if (material.alpha.x == 1.0 && albedo.w < material.alpha.y) discard;
  
  albedo *= material.base_color;
  albedo *= var.color;

  ivec2 albedo_size = textureSize(tex_sampler[DIFFUSE_INDEX], 0);
  if (albedo_size.x == 1 && albedo_size.y == 1) {
    albedo = material.base_color;
  }

  vec4 C_light = vec4(sun_light.color, 0.0) * 0.1;
  vec4 C_ambient = material.ambient * albedo * C_light * ao;
  vec4 C_diffuse = material.diffuse * albedo / PI;
  vec4 C_specular = material.specular;

  vec3 p = var.t_frag_pos;
  vec3 v = var.t_view_dir;
  
  vec3 F0 = mix(vec3(0.04), albedo.xyz, metallic);//normal incidence
  float P =  5.0 * (1.0 - roughness); 

  vec4 out_lights = vec4(0.0, 0.0, 0.0, 1.0);
  for (int i = 1; i < NR_POINT_LIGHTS; ++i) {

    vec3 light_pos = var.t_plight_pos[i];
    vec3 l = normalize(light_pos - p);

    //float distance = length(point_lights[i].position - var.frag_pos);
    //float attenuation = 1.0 / (distance * distance);
    float attenuation = SmoothAttenuation(l);
    vec3 l_color = point_lights[i].color;
    C_light = vec4(l_color * attenuation, 1.0);
    vec3 F90 = mix(F0, vec3(1.0), metallic);//vec3(1.0); //point_lights[i].color;
    //C_light = vec4(point_lights[i].color, 1.0);

    //Fresnel
    float NdL = max(dot(normal, l), 0.0);
    float NdL2 = NdL*NdL;
    //GGX distribution
    //@todo do anisotropic
    //Not anisotropic
    vec3 h = normalize(l + v);
    float NdH = dot(normal, h);
    float NdH2 = NdH*NdH;
    float NdV = dot(normal, v);
    float NdV2 = NdV*NdV;
    float HdV = dot(h, v);
    float HdV2 = HdV*HdV;
    float HdL = dot(h, l);
    float HdL2 = HdL*HdL;

    //Fresnel reflectance
    vec3 bounce = (20.0 / 21.0) * F0 + (1.0 / 21.0);
    vec3 F = bounce + (F90 - bounce) * pow(max(1.0 - NdL, 0.0), 1.0/P);
    //vec3 F = F0 + (F90 - F0) * pow(max(1.0 - NdL, 0.0), 1/P);
    //vec3 F = F0 + (1 - F0) * pow(max(1.0 - NdL, 0.0), 5);

    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);

    //masking
    float delta_mv = sqrt(roughness2 + (1.0 - roughness2) * HdV2);
    float delta_ml = sqrt(roughness2 + (1.0 - roughness2) * HdL2);
//    float delta_nv = (1 - sqrt(1 + (1.0 / roughness2))) / 2;
//    float delta_nl = sqrt(roughness2 + (1.0 - roughness2) * HdL2);

    float chi = (HdV > 0.0) ? 1.0 : 0.0;
    float chi2 = (HdL > 0.0) ? 1.0 : 0.0;
    float G1 = chi / (1 + delta_mv);
    float G2 = (chi * chi2) / (1 + delta_mv + delta_ml);

    //float G1 = NdL / (NdL * (2.0 - roughness2) + roughness2);
    //float G2 = 0.5 / mix(2.0 * NdL * NdV, NdL + NdV, roughness2);
    //float G2 = 0.5 * (1.0 + (2.0 * NdL * NdV) / (NdL + NdV + roughness2));

    float tmp = PI * (1.0 + NdH2 * (roughness2 - 1.0));
    float NDF = (roughness2) / (PI * (tmp * tmp));

    vec3 specular = (NDF * (G1 * G2) * F) / ((4.0 * max(NdV, 0.0) * max(NdL, 0.0)) + 0.0001) ;
    
    //vec3 diffuse = (21.0 / (20.0 * PI))  * (C_diffuse.xyz * (1.0 - F)) *  (1.0 - pow(1.0 - NdL, 5.0)) * (1.0 - pow(1.0 - NdH, 5.0));
    vec3 diffuse = (1 - F) * C_diffuse.xyz;
    out_lights += vec4((kD * diffuse + specular) * C_light.xyz * NdL , 1.0);
  }
  
  vec4 color = C_ambient + out_lights;
  color *= PI;

  vec2 emissive_coord = transform_uv(
    material.emissive_translation,
    material.emissive_scale,
    material.emissive_rotation,
    var.texture_coord); 

  vec4 emissive_color = texture(tex_sampler[EMISSIVE_INDEX], -emissive_coord);
  ivec2 emissive_color_size = textureSize(tex_sampler[EMISSIVE_INDEX], 0);
  if (emissive_color_size.x != 1 && emissive_color_size.y != 1) {
    color += material.emission * emissive_color;
  }

  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);

  final_color = color;

//@todo post process for alpha blending
//this is just to show where alpha blending is needed (same color as the skybox?)
  if (material.alpha.x == 2.0) {
    final_color = vec4(color.xyz, color_alpha);
  }
}
