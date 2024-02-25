#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    flat int fTextureID;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    //x bump map, y specular map, z alpha, w blank
    vec4 fMapsUsed;
    vec4 fViewPos;
    mat3 TBN;
    vec4 fTangent;
    vec3 fBitangent;
    vec4 fShadowCoordAmbient;
    vec4 fShadowCoordSpot;
    //faceId texture ID blank blank
    vec4 ffidtidBB;
    vec3 fvColor;
    vec3 viewT;
    vec3 lightT;
    vec3 posT;
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

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow, vec3 lightDir, vec2 texCoord);
vec3 CalcPointLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 lightDir, vec2 texCoord);
vec3 CalcSpotLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec3 lightDir, vec2 texCoord);
float ShadowCalculation(vec4 shadowCoord, vec2 off);
float filterPCF(vec4 sc);
vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir, int id);
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

    vec3 normal = normalize(fs_in.fNormal);
    vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
    vec3 lightDir = normalize(ambientLight.position - fs_in.fPos.xyz);
    vec3 lPointLightTan[NR_POINT_LIGHTS];
    vec3 spotLightDir = normalize(spotLight.position - fs_in.fPos);
    vec2 texCoord = fs_in.fTexCoord;

    int id = 0;
    //@todo ugly but avoid floating point issues when casting to int
    if (fs_in.ffidtidBB.y > 0.0 && fs_in.ffidtidBB.y < 0.2) {
      id = 1;
    } else if (fs_in.ffidtidBB.y > 0.1 && fs_in.ffidtidBB.y < 0.3) {
      id = 2;
    }

    vec4 texColor = texture(texSampler[id], texCoord);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
      lPointLightTan[i] = vec3(1.0);
    }
    
    if (fs_in.fMapsUsed.x > 0.9 && fs_in.fMapsUsed.x < 1.1) {
      //vec3 vdir = normalize(fs_in.viewT - fs_in.posT);
      //viewDir = normalize(fs_in.viewT);
      //lightDir = normalize(fs_in.lightT);

      //texCoord = ParallaxMapping(fs_in.fTexCoord, vdir, id);

      vec3 n = normalize(fs_in.fNormal);
      vec2 tmpM = texture(texSampler2[1], texCoord).xy;

      vec3 m = vec3(tmpM, sqrt(1.0 - tmpM.x * tmpM.x - tmpM.y * tmpM.y));
      vec3 t = normalize(fs_in.fTangent.xyz - n * dot(fs_in.fTangent.xyz, n));
      vec3 b = cross(n, fs_in.fTangent.xyz) * fs_in.fTangent.w;
      normal = normalize(t * m.x + b * m.y + n * m.z);

//      texColor = texture(texSampler[id], texCoord);
    } 

    if (fs_in.fMapsUsed.w > 0.9 && fs_in.fMapsUsed.w < 1.1) {
      texColor = vec4(fs_in.fvColor, 1.0);
    }

    //float shadowAmbient = ShadowCalculation(fs_in.fShadowCoordAmbient / fs_in.fShadowCoordAmbient.w, vec2(0.0));
    float shadowAmbient = filterPCF(fs_in.fShadowCoordAmbient / fs_in.fShadowCoordAmbient.w);
    //float shadowSpot = filterPCF(fs_in.fShadowCoordAmbient / fs_in.fShadowCoordAmbient.w);

    //vec3 pixelColor = CalcDirLight(texColor, ambientLight, normal, viewDir, shadowAmbient, lightDir, texCoord);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        //vec3 lightDir = normalize(pointLights[i].position - fs_in.fPos) * lPointLightTan[i];

        //pixelColor += CalcPointLight(texColor, pointLights[i], normal, fs_in.fPos.xyz, viewDir, lightDir, texCoord);
    }

    //pixelColor += CalcSpotLight(texColor, spotLight, normal, fs_in.fPos.xyz, viewDir, shadowAmbient, spotLightDir, texCoord);

    if (fs_in.fMapsUsed.z > 0.9 && fs_in.fMapsUsed.z < 1.1) {
        vec4 mask = texture(texSampler[3], texCoord);
//        fColor = mix(fColor, mask, mask.a);
        if (mask.r < 0.7) discard;
    }
    if (texColor.a < 0.7) discard;


    //pixelColor = normal.xyz;
    vec3 pixelColor = normal;
    fColor = vec4(pixelColor, 1.0);

    //fColor = vec4(fs_in.fShadowCoord.st, 0, 1);
    //float depthValue = texture(texSampler[4], fs_in.fTexCoord).r;
    //fColor = vec4(vec3(depthValue), 1.0);
}

vec3 CalcDirLight(vec4 color, Light dirLight, vec3 normal, vec3 viewDir, float shadow, vec3 lightDir, vec2 texCoord)
{
    vec3 ambient = GetAmbientLight();
    viewDir = normalize(-viewDir);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse =  dirLight.color * material.diffuse * diff;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.9 && fs_in.fMapsUsed.y < 1.1) {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular = dirLight.color * (spec * dirLight.ads.z * material.specular * texture(texSampler2[0], texCoord).xyz);
    } else {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular =  dirLight.color * (dirLight.ads.z * spec * material.specular);
    }

    return (ambient + (diffuse + specular)) * color.xyz;
}

vec3 CalcPointLight(vec4 color, Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 lightDir, vec2 texCoord)
{
    vec3 ambient = GetAmbientLight();

    float distance = length(pointLight.position - fs_in.fPos);
    float attenuation = 1.0 / (pointLight.clq.x + pointLight.clq.y * distance + pointLight.clq.z * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = pointLight.color * material.diffuse * diff * pointLight.ads.y;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.9 && fs_in.fMapsUsed.y < 1.1) {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular =  pointLight.color * (spec * pointLight.ads.z * material.specular * texture(texSampler2[0], texCoord).xyz);
    } else {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular = pointLight.color * (pointLight.ads.z * spec * material.specular);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * color.xyz;
}

vec3 CalcSpotLight(vec4 color, Light spotlight, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow, vec3 lightDir, vec2 texCoord)
{
    vec3 ambient = GetAmbientLight();

    float theta = dot(lightDir, normalize(-spotlight.direction));
    float epsilon   = spotlight.coB.x - spotlight.coB.y;
    float intensity = clamp((theta - spotlight.coB.y) / epsilon, 0.0, 1.0);

    float distance = length(spotlight.position - fs_in.fPos);
    float attenuation = 1.0 / (spotlight.clq.x + spotlight.clq.y * distance + spotlight.clq.z * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = spotlight.color * material.diffuse * diff * spotlight.ads.y;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.9 && fs_in.fMapsUsed.y < 1.1) {
        float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
        specular = spotlight.color * (spec * spotlight.ads.z * material.specular * texture(texSampler2[0], texCoord).xyz);
    } else {
        float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
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

float filterPCF(vec4 shadowCoord)
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

vec2 ParallaxMapping(vec2 texCoord, vec3 viewDir, int id)
{
    int scale = 24;
    int k = 4;
    vec2 texSize = textureSize(texSampler[id], 1);
    vec2 u = vec2(scale / (2 * texSize.x * k), scale / (2 * texSize.y * k));

    vec2 pdir = viewDir.xy * u;
    for (int i = 0; i < k; ++i)
    {
      float parallax = texture(texSampler2[2], texCoord).x;
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
