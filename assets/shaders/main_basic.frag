#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 2

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
  flat int fTextureID;
  vec2 fTexCoord;
  vec3 fNormal;
  vec3 fPos;
  vec4 fViewPos;
  mat3 TBN;
  vec4 fAmbientLightSpace;
  vec4 fShadowCoordSpot;
  //faceId texture ID blank blank
  vec4 ffidtidBB;
  vec3 fvColor;
} fs_in;

struct Light {
  vec3 color;
  vec3 direction;
  vec3 position;
  //ambiance diffuse specular
  vec3 ads;
  //constant, linear, quadratiq
  vec3 clq;
  //cutOff, outerCutoff, Blank
  vec3 coB;
  mat4 view;
  mat4 projection;
  mat4 lightSpaceMatrix;
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
layout(binding = 1) uniform sampler2D texSampler[5];

layout(binding = 2) readonly buffer ObjectBuffer {
  Light ambientLight;
  Light pointLights[NR_POINT_LIGHTS];
  Light spotLight;
  Material material;
};

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow);
vec3 CalcPointLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
float ShadowCalculation(vec4 lightSpace);

float near = 0.1;
float far  = 100.0;

float LinearizeDepth(float depth)
{
  return (2.0 * near * far) / (far + near - depth * (far - near));
}

void main()
{
  //vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
  //vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient
  //vec3 debugSpecular = vec3(0.0, 0.0, 1.0); // Debugging color for specular

  vec3 normal = normalize(fs_in.fNormal);

//  ivec2 texBumMapSize = textureSize(texSampler[3], 0);
//
//  if (texBumMapSize.x != 1 && texBumMapSize.y != 1) {
//    vec3 nm = texture(texSampler[3], fs_in.fTexCoord).xyz * 2.0 - vec3(1.0);
//    nm = fs_in.TBN * nm;
//    normal = vec3(normalize(nm));
//  }

  vec4 texColor = texture(texSampler[0], fs_in.fTexCoord);
 
  //float shadowAmbient = 1.0;//1 not in shadow, 0 in shadow
  float shadowAmbient = ShadowCalculation(fs_in.fAmbientLightSpace);

  vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
  vec3 color = CalcDirLight(texColor, ambientLight, normal, viewDir, shadowAmbient);

//  for(int i = 0; i < NR_POINT_LIGHTS; i++) {
//    color += CalcPointLight(texColor, pointLights[i], normal, fs_in.fPos.xyz, viewDir);
//  }

//  color += CalcSpotLight(texColor, spotLight, normal, fs_in.fPos.xyz, viewDir, 1.0);

  ivec2 texMaskSize = textureSize(texSampler[1], 0);
  if (texMaskSize.x != 1 && texMaskSize.y != 1) {
    vec4 mask = texture(texSampler[1], fs_in.fTexCoord);
//        fColor = mix(fColor, mask, mask.a);
    if (mask.r < 0.2) discard;
  }
  if (texColor.a < 0.1) discard;

  fColor = vec4(color, 1.0);
}

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow)
{
  vec3 lightDir = normalize(-dirLight.direction - fs_in.fPos);
  vec3 ambient = dirLight.color * (material.ambient);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = dirLight.color * (diff * material.diffuse);

  vec3 h = normalize(lightDir + viewDir);
  vec3 specular = vec3(0.0);

//  float spec = 0.0;
//  spec = pow(max(dot(normal, h), 0.0), 1.0);
//  specular = spec * dirLight.color;
//
//    ivec2 texSize = textureSize(texSampler[4], 0);

//    if (texSize.x == 1 && texSize.y == 1) {
    //float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
    //specular =  dirLight.color * (dirLight.ads.z * spec * material.specular);
//    } else {
    float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shi_ior_diss.r) * float(dot(normal, h) > 0.0);
    specular = dirLight.color * (spec) * texture(texSampler[2], fs_in.fTexCoord).xyz;
//    }

  return (ambient + (1 - shadow) * (diffuse + specular)) * color.xyz;
}

vec3 CalcPointLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(pointLight.position - fs_in.fPos);
  vec3 ambient = pointLight.color * material.ambient * pointLight.ads.x;

  float distance = length(pointLight.position - fs_in.fPos);
  float attenuation = 1.0 / (pointLight.clq.x + pointLight.clq.y * distance + pointLight.clq.z * (distance * distance));

 float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = pointLight.color * (diff * material.diffuse);

  vec3 h = normalize(lightDir + viewDir);
  vec3 specular = vec3(0.0);

  float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shi_ior_diss.r) * float(dot(normal, h) > 0.0);
  specular = pointLight.color * (spec * material.specular.r) * texture(texSampler[2], fs_in.fTexCoord).xyz;

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return (ambient + diffuse + specular) * color.xyz;
}

vec3 CalcSpotLight(vec4 color, Light spotlight, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
  vec3 lightDir = normalize(spotlight.position - fs_in.fPos);
  vec3 ambient = spotlight.color * material.ambient * spotlight.ads.x;

  float theta = dot(lightDir, normalize(-spotlight.direction));
  float epsilon   = spotlight.coB.x - spotlight.coB.y;
  float intensity = clamp((theta - spotlight.coB.y) / epsilon, 0.0, 1.0);

  float distance = length(spotlight.position - fs_in.fPos);
  float attenuation = 1.0 / (spotlight.clq.x + spotlight.clq.y * distance + spotlight.clq.z * (distance * distance));

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = spotlight.color * material.diffuse * diff * spotlight.ads.y;

  vec3 h = normalize(lightDir + viewDir);
  vec3 specular = vec3(0.0);

  ivec2 texSize = textureSize(texSampler[4], 0);

  if (texSize.x == 1 && texSize.y == 1) {
    float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shi_ior_diss.x) * float(dot(normal, h) > 0.0);
    specular = spotlight.color * (spotlight.ads.z * spec * material.specular);
  } else {
    float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shi_ior_diss.x) * float(dot(normal, h) > 0.0);
    specular = spotlight.color * (spec * spotlight.ads.z * material.specular * texture(texSampler[2], fs_in.fTexCoord).xyz);
  }

  ambient *= attenuation * intensity;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;

  return (ambient + shadow * (diffuse + specular)) * color.xyz;
}

float ShadowCalculation(vec4 ambientLightSpace)
{
  vec3 coord = ambientLightSpace.xyz / ambientLightSpace.w;
  //coord = coord * 0.5 + 0.5;

  float closestDepth = texture(texSampler[4], coord.xy).r;
  float currentDepth = coord.z;
  float bias = max(0.05 * (1.0 - dot(fs_in.fNormal, -ambientLight.direction)), 0.005);

  float shadow = 0.1;
  vec2 texelSize = textureSize(texSampler[4], 0);
  int scale = 1;
  for(int x = -scale; x <= scale; ++x)
  {
    for(int y = -scale; y <= scale; ++y)
    {
      float pcfDepth = texture(texSampler[4], coord.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  if(coord.z > 1.0)
      shadow = 0.0;
    


  return shadow;
}