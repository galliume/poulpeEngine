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
  vec4 fAmbientLightSpace;
  vec4 fShadowCoordSpot;
  //faceId texture ID blank blank
  vec4 ffidtidBB;
  vec3 fvColor;
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
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
float ShadowCalculation(vec4 lightSpace, vec3 normal);

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
  ivec2 texSize = textureSize(texSampler[3], 0);

  if (texSize.x != 1 && texSize.y != 1) {
    normal = texture(texSampler[3], fs_in.fTexCoord).xyz;
    normal = normalize(normal * 2.0 - 1.0);
  }

  vec4 texColor = texture(texSampler[0], fs_in.fTexCoord);
 
  //float shadowAmbient = 1.0;//1 not in shadow, 0 in shadow
  float shadowAmbient = ShadowCalculation(fs_in.fAmbientLightSpace, normal);

  vec3 viewDir = normalize(fs_in.TangentViewPos.xyz - fs_in.TangentFragPos.xyz);
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


  color = color / (color + vec3(1.0));
  //color = vec3(-shadowAmbient);
  color = pow(color, vec3(1.0 / 2.2));
  
  fColor = vec4(color, 1.0);
}

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow)
{
  vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
  vec3 ambient = (dirLight.color * vec3(texture(texSampler[0], fs_in.fTexCoord))) * dirLight.ads.x;

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse =  (diff * vec3(texture(texSampler[0], fs_in.fTexCoord))) * dirLight.ads.y;

  //vec3 reflectDir = reflect(-dirLight.position, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 1);

  vec3 specular = vec3(1.f);
  ivec2 texSize = textureSize(texSampler[2], 0);

  if (texSize.x == 1 && texSize.y == 1) {
    specular = (material.specular * spec) * dirLight.color * dirLight.ads.z;
  } else {
    specular = (vec3(texture(texSampler[2], fs_in.fTexCoord)) * spec) * dirLight.color * dirLight.ads.z;
  }

  return (ambient +   (diffuse + specular)) * color.xyz;
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
  vec3 lightDir = normalize(spotlight.direction - fs_in.fPos);
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

  return ((ambient + diffuse + specular) * shadow) * color.xyz;
}

float ShadowCalculation(vec4 ambientLightSpace, vec3 normal)
{
  vec4 coord = ambientLightSpace / ambientLightSpace.w;
  //coord.y = 1.0 - coord.y;
  ivec2 texDim = textureSize(texSampler[4], 0);
  float scale = 1.5;
  float dx = 1.0 / float(texDim.x);
  float dy = 1.0 / float(texDim.y);

  float shadow = 0.0;
  int count = 0;
  int range = 1;
  vec3 lightDir = normalize(ambientLight.position - fs_in.fPos);
  float bias = max(0.05 * (1.0 - dot(normalize(normal), lightDir)), 0.005);

  for (int x = -range; x <= range; x++)
  {
    for (int y = -range; y <= range; y++)
    {
      float pcfDepth = texture(texSampler[4], coord.xy + vec2(x*dx, y*dy)).x;
      shadow += coord.z - bias > pcfDepth ? 1.0 : 0.0;
      count++;
    }
  
  }
  shadow /= count;

  if(coord.z > 1.0) {
    shadow = 0.0;
  }

  return (1.0 - shadow);
}