#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    flat int fTextureID;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec3 fMapsUsed;
    vec4 fViewPos;
    mat3 TBN;
    vec4 fShadowCoord;
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

#define NR_POINT_LIGHTS 2
layout(binding = 2) buffer ObjectBuffer {
    Light ambientLight;
    Light pointLights[NR_POINT_LIGHTS];
    Light spotLight;
    Material material;
};

vec3 CalcDirLight(Light dirLight, vec3 normal, vec3 viewDir, float shadow);
vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 shadowCoord);

float near = 0.1;
float far  = 100.0;

float LinearizeDepth(float depth)
{
    return (2.0 * near * far) / (far + near - depth * (far - near));
}

void main()
{
    vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
    vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient
    vec3 debugSpecular = vec3(0.0, 0.0, 1.0); // Debugging color for specular

//    if (texture(texSampler[0], fs_in.fTexCoord).a < 0.5) {
//        discard;
//    }

    vec3 normal = normalize(fs_in.fNormal);

    if (fs_in.fMapsUsed.x > 0.5) {
      vec3 nm = texture(texSampler[2], fs_in.fTexCoord).xyz * 2.0 - vec3(1.0);
      nm = fs_in.TBN * nm;
      normal = vec3(normalize(nm));
    }

    float shadow = ShadowCalculation(fs_in.fShadowCoord / fs_in.fShadowCoord.w);

    vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
    vec3 color = CalcDirLight(ambientLight, normal, viewDir, shadow);
    
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        color += CalcPointLight(pointLights[i], normal, fs_in.fPos.xyz, viewDir);
    }

    color += CalcSpotLight(spotLight, normal, fs_in.fPos.xyz, viewDir);

    fColor = vec4(color, 1.0);
    //fColor = vec4(fs_in.fShadowCoord.st, 0, 1);
    //float depthValue = texture(texSampler[3], fs_in.fTexCoord).r;
    //fColor = vec4(vec3(depthValue), 1.0);
}

vec3 CalcDirLight(Light dirLight, vec3 normal, vec3 viewDir, float shadow)
{
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 ambient = dirLight.color * material.ambient * dirLight.ads.x;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse =  dirLight.color * material.diffuse * diff * dirLight.ads.y;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.5) {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular = dirLight.color * (spec * dirLight.ads.z * material.specular * texture(texSampler[1], fs_in.fTexCoord).xyz);
    } else {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular =  dirLight.color * (dirLight.ads.z * spec * material.specular);
    }

    return (ambient + shadow * (diffuse + specular)) * texture(texSampler[0], fs_in.fTexCoord).xyz;
}

vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(pointLight.position - fs_in.fPos);
    vec3 ambient = pointLight.color * material.ambient * pointLight.ads.x;

    float distance = length(pointLight.position - fs_in.fPos);
    float attenuation = 1.0 / (pointLight.clq.x + pointLight.clq.y * distance + pointLight.clq.z * (distance * distance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = pointLight.color * material.diffuse * diff * pointLight.ads.y;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.5) {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular =  pointLight.color * (spec * pointLight.ads.z * material.specular * texture(texSampler[1], fs_in.fTexCoord).xyz);
    } else {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
      specular = pointLight.color * (pointLight.ads.z * spec * material.specular);
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular) * texture(texSampler[0], fs_in.fTexCoord).xyz;
}

vec3 CalcSpotLight(Light spotlight, vec3 normal, vec3 fragPos, vec3 viewDir)
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

    if (fs_in.fMapsUsed.y > 0.5) {
        float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
        specular = spotlight.color * (spec * spotlight.ads.z * material.specular * texture(texSampler[1], fs_in.fTexCoord).xyz);
    } else {
        float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
        specular = spotlight.color * (spotlight.ads.z * spec * material.specular);
    }

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular) * texture(texSampler[0], fs_in.fTexCoord).xyz;
}

float ShadowCalculation(vec4 shadowCoord)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = texture(texSampler[3], shadowCoord.st + vec2(0.0f)).r;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.0;
        }
    }

    return shadow;
}
