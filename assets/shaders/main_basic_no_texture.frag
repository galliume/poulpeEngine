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

layout(location = 0) out vec4 fColor;

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

float near = 0.1;
float far  = 100.0;
  
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec3 CalcDirLight(Light dirLight, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 color = vec3(1.0, 1.0, 1.0);

void main()
{
	
    vec3 normal = normalize(var.norm);

    vec3 viewDir = normalize(var.view_pos.xyz - var.frag_pos.xyz);
    vec3 lights = CalcDirLight(sun_light, normal, viewDir);
    
    //for(int i = 0; i < NR_POINT_LIGHTS; i++) {
    //    lights += CalcPointLight(pointLights[i], normal, var.frag_pos.xyz, viewDir);
    //}

    //
	//lights += CalcSpotLight(spotLight, normal, var.frag_pos.xyz, viewDir);

    fColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

vec3 CalcDirLight(Light dirLight, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 ambient = dirLight.color * material.ambient.rgb * dirLight.ads.x;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse =  dirLight.color * material.diffuse.rgb * diff * dirLight.ads.y;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shi_ior_diss.x) * float(dot(normal, h) > 0.0);
    specular =  dirLight.color * (dirLight.ads.z * spec * material.specular.rgb);

    return vec3(0, 1, 0) ;
}

vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(pointLight.position - var.frag_pos);
    vec3 ambient = pointLight.ads.x * pointLight.color * color;

    float distance = length(pointLight.position - var.frag_pos);
    float attenuation = 1.0 / (pointLight.clq.x + pointLight.clq.y * distance + pointLight.clq.z * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * pointLight.ads.y * color;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(1.0);

    float spec = pow(clamp(dot(normal, h), 0.0, 1.0), 1.0) * float(dot(normal, h) > 0.0);
    specular *= pointLight.ads.z * spec * color;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Light spotlight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(spotlight.position - var.frag_pos);
    vec3 ambient = spotlight.ads.x * spotlight.color * color;

    float theta = dot(lightDir, normalize(-spotlight.direction));
    float epsilon   = spotlight.coB.z - spotlight.coB.y;
    float intensity = clamp((theta - spotlight.coB.y) / epsilon, 0.0, 1.0);

    float distance = length(spotlight.position - var.frag_pos);
    float attenuation = 1.0 / (spotlight.clq.x + spotlight.clq.y * distance + spotlight.clq.z * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * spotlight.ads.y * color;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(1.0);

	float spec = pow(clamp(dot(normal, h), 0.0, 1.0), 1.0) * float(dot(normal, h) > 0.0);
	specular *= spotlight.ads.z * spec * color;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
