#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    flat int fTextureID;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec4 fMapsUsed;
    vec4 fViewPos;
    vec4 fTangent;
    vec3 fBitangent;
    vec4 fShadowCoordAmbient;
    vec4 fShadowCoordSpot;
    //faceId texture ID blank blank
    vec4 ffidtidBB;
    vec3 fvColor;
    mat4 viewMatrix;
    mat4 modelMatrix;
    mat4 viewModelMatrix;
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
    vec3 shiIorDiss;
};

layout(binding = 1) uniform sampler2D texSampler[4];
layout(binding = 2) uniform sampler2D texSampler2[4];

#define NR_POINT_LIGHTS 2
layout(binding = 3) buffer ObjectBuffer {
    Light ambientLight;
    Light pointLights[NR_POINT_LIGHTS];
    Light spotLight;
    Material material;
};

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow, vec2 texCoord);
vec3 CalcPointLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoord);
vec3 CalcSpotLight(vec4 color, Light spotLight, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec2 texCoord);

float FilterPCF(vec4 sc);
float ShadowCalculation(vec4 shadowCoord, vec2 off);

vec2 ParallaxMapping(vec2 texCoord, vec3 vDir, int id);

vec3 FromObjectSpaceToWorldSpace(vec3 vec);
vec3 FromObjectSpaceToViewSpace(vec3 vec);

vec3 GetAmbientLight();

float near = 0.1;
float far  = 100.0;
int scale = 24;

float LinearizeDepth(float depth)
{
    return (2.0 * near * far) / (far + near - depth * (far - near));
}

void main()
{
    vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
    vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient
    vec3 debugSpecular = vec3(0.0, 0.0, 1.0); // Debugging color for specular

    vec3 viewPos = fs_in.fViewPos.xyz;
    vec3 fragPos = fs_in.fPos;
    vec3 normal  = fs_in.fNormal;
    
    vec3 viewDir = normalize(viewPos - fragPos);
//    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
//      pointLights[i].position = vec3(fs_in.model * vec4(pointLights[i].position, 0.0));
//    }
    
    vec2 texCoord = fs_in.fTexCoord;

    int id = 0;
    //@todo ugly but avoid floating point issues when casting to int
    if (fs_in.ffidtidBB.y > 0.0 && fs_in.ffidtidBB.y < 0.2) {
      id = 1;
    } else if (fs_in.ffidtidBB.y > 0.1 && fs_in.ffidtidBB.y < 0.3) {
      id = 2;
    }

    vec4 texColor = texture(texSampler[id], texCoord);
    
    if (fs_in.fMapsUsed.x > 0.9 && fs_in.fMapsUsed.x < 1.1) {
//      vec2 tmpM = texture(texSampler2[1], texCoord).xy * 0.5 + 0.5;
//      vec3 m = vec3(tmpM, sqrt(1.0 - tmpM.x * tmpM.x - tmpM.y * tmpM.y));
//      m = normalize(m);
//
//    vec3 tangent = normalize(fs_in.fTangent.xyz - normal * dot(fs_in.fTangent.xyz, normal));
//    vec3 bitangent = cross(fs_in.fNormal, fs_in.fTangent.xyz) * fs_in.fTangent.w;
//    normal = normalize(tangent * m.x + bitangent * m.y + normal * m.z);
      //spotLightDir = normalize(spotLightDir * TBN);
//      
//      for(int i = 0; i < NR_POINT_LIGHTS; i++) {
//        pointLights[i].position = normalize(pointLights[i].position * TBN);
//      }

//      viewDir = normalize(tangent * viewDir.x + bitangent * viewDir.y + normal * viewDir.z);
//
//      texCoord = ParallaxMapping(fs_in.fTexCoord, viewDir, id);
//      texColor = texture(texSampler[id], texCoord);
    }

    if (fs_in.fMapsUsed.w > 0.9 && fs_in.fMapsUsed.w < 1.1) {
      texColor = vec4(fs_in.fvColor, 1.0);
    }

    //float shadowAmbient = ShadowCalculation(fs_in.fShadowCoordAmbient / fs_in.fShadowCoordAmbient.w, vec2(0.0));
    float shadowAmbient = 1.0;//FilterPCF(fs_in.fShadowCoordAmbient / fs_in.fShadowCoordAmbient.w);
    //float shadowSpot = filterPCF(fs_in.fShadowCoordAmbient / fs_in.fShadowCoordAmbient.w);

    //vec3 pixelColor = CalcDirLight(texColor, ambientLight, normal, viewDir, shadowAmbient, texCoord);

    //for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        //pixelColor += CalcPointLight(texColor, pointLights[i], normal, fs_in.fPos.xyz, viewDir, texCoord);
    //}

    vec3 pixelColor = CalcSpotLight(texColor, spotLight, normal, viewPos, viewDir, shadowAmbient, texCoord);

    if (fs_in.fMapsUsed.z > 0.9 && fs_in.fMapsUsed.z < 1.1) {
        vec4 mask = texture(texSampler[3], texCoord);
//        fColor = mix(fColor, mask, mask.a);
        if (mask.r < 0.7) discard;
    }
    if (texColor.a < 0.7) discard;


    //pixelColor = normal.xyz;
    fColor = vec4(pixelColor, 1.0);

    //fColor = vec4(fs_in.fShadowCoord.st, 0, 1);
    //float depthValue = texture(texSampler[4], fs_in.fTexCoord).r;
    //fColor = vec4(vec3(depthValue), 1.0);
}

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow, vec2 texCoord)
{
    vec3 ambient = GetAmbientLight();

    vec3 n = FromObjectSpaceToViewSpace(normal);
    vec3 l = FromObjectSpaceToViewSpace(-dirLight.direction);

    float diff = max(dot(n, l), 0.0);
    vec3 diffuse =  dirLight.color * material.diffuse * diff;

    vec3 h = normalize(l + FromObjectSpaceToViewSpace(viewDir));
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.9 && fs_in.fMapsUsed.y < 1.1) {
      float spec = pow(clamp(dot(n, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(n, h) > 0.0);
      specular = dirLight.color * (spec * dirLight.ads.z * material.specular * texture(texSampler2[0], texCoord).xyz);
    } else {
      float spec = pow(clamp(dot(n, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(n, h) > 0.0);
      specular =  dirLight.color * (dirLight.ads.z * spec * material.specular);
    }

    return (ambient + shadow * (diffuse + specular)) * color.xyz;
}

vec3 CalcPointLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoord)
{
    vec3 ambient = GetAmbientLight();
    ambient += pointLight.color * material.ambient * pointLight.ads.x;

    vec3 n = FromObjectSpaceToViewSpace(normal);
    vec3 l = FromObjectSpaceToViewSpace(pointLight.position);

    float distance = length(l - FromObjectSpaceToViewSpace(fs_in.fPos));
    float attenuation = 1.0 / (pointLight.clq.x + pointLight.clq.y * distance + pointLight.clq.z * (distance * distance));

    float diff = max(dot(n, l), 0.0);
    vec3 diffuse = pointLight.color * material.diffuse * diff * pointLight.ads.y;

    vec3 h = normalize(l + FromObjectSpaceToViewSpace(viewDir));
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.9 && fs_in.fMapsUsed.y < 1.1) {
      float spec = pow(clamp(dot(n, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(n, h) > 0.0);
      specular =  pointLight.color * (spec * pointLight.ads.z * material.specular * texture(texSampler2[0], texCoord).xyz);
    } else {
      float spec = pow(clamp(dot(n, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(n, h) > 0.0);
      specular = pointLight.color * (pointLight.ads.z * spec * material.specular);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * color.xyz;
}

vec3 CalcSpotLight(vec4 color, Light spotlight, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec2 texCoord)
{
    vec3 ambient = GetAmbientLight();
    ambient += spotlight.color * material.ambient * spotlight.ads.x;

    vec3 n = FromObjectSpaceToViewSpace(normal);
    vec3 l = FromObjectSpaceToViewSpace(spotlight.position - fs_in.fPos);
    vec3 d = FromObjectSpaceToViewSpace(spotlight.direction);

    float theta = dot(l, d);
    float epsilon   = spotlight.coB.x - spotlight.coB.y;
    float intensity = clamp((theta - spotlight.coB.y) / epsilon, 0.0, 1.0);

    float distance = length(FromObjectSpaceToViewSpace(spotlight.position) - FromObjectSpaceToViewSpace(fs_in.fPos));
    float attenuation = 1.0 / (spotlight.clq.x + spotlight.clq.y * distance + spotlight.clq.z * (distance * distance));

    float diff = max(dot(n, l), 0.0);
    vec3 diffuse = spotlight.color * material.diffuse * diff * spotlight.ads.y;

    vec3 h = normalize(l + FromObjectSpaceToViewSpace(viewDir));
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.9 && fs_in.fMapsUsed.y < 1.1) {
        float spec = pow(clamp(dot(n, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(n, h) > 0.0);
        specular = spotlight.color * (spec * spotlight.ads.z * material.specular * texture(texSampler2[0], texCoord).xyz);
    } else {
        float spec = pow(clamp(dot(n, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(n, h) > 0.0);
        specular = spotlight.color * (spotlight.ads.z * spec * material.specular);
    }

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + shadow * (diffuse + specular)) * color.xyz;
}

float ShadowCalculation(vec4 shadowCoord, vec2 off)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = texture(texSampler2[3], shadowCoord.st + off).r;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.1;
        }
    }
    return shadow;
}

float FilterPCF(vec4 shadowCoord)
{
    ivec2 texDim = textureSize(texSampler2[3], 0);
    float scale = 1.5;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;
    
    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            shadowFactor += ShadowCalculation(shadowCoord, vec2(dx*x, dy*y));
            count++;
        }
    
    }
    return shadowFactor / count;
}

vec2 ParallaxMapping(vec2 texCoord, vec3 vDir, int id)
{
    int scale = 24;
    int k = 4;
    vec2 texSize = textureSize(texSampler2[2], 1);
    vec2 u = vec2(scale / (2 * (texSize.x / 2) * k), scale / (2 * (texSize.y / 2) * k));

    vec2 pdir = vDir.xy * u;
    for (int i = 0; i < k; ++i)
    {
      float parallax = normalize(texture(texSampler2[2], texCoord) * 0.5 + 0.5).x;
      texCoord += pdir * parallax;
    }
    return texCoord;
//    float height_scale = 0.1;
//    float height =  texture(texSampler2[2], texCoord).x;
//    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
//    return texCoord - p; 
}

vec3 GetAmbientLight()
{
    return ambientLight.color * material.ambient * ambientLight.ads.x;
}

vec3 FromObjectSpaceToWorldSpace(vec3 vec)
{
  return normalize(fs_in.modelMatrix * vec4(vec, 0.0)).xyz;
}

//i.e Camera Space
vec3 FromObjectSpaceToViewSpace(vec3 vec)
{
  return normalize(fs_in.viewModelMatrix * vec4(vec, 0.0)).xyz;
}
