#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    flat int fTextureID;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec4 fViewPos;
    mat3 TBN;
} fs_in;

struct Light {
    vec3 color;
    vec3 direction;
    vec3 position;
    //ambiance diffuse specular
    vec3 ads;
    //constant, linear, quadratiq
    vec3 clq;
    //cutOff, outerCuoffBlank
    vec3 coB;
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

layout(binding = 1) uniform sampler2D texSampler[3];

#define NR_POINT_LIGHTS 2
layout(binding = 2) buffer ObjectBuffer {
    Light ambientLight;
    Light pointLights[NR_POINT_LIGHTS];
    Light spotLight;
    Material material;
};

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
	
    vec3 normal = normalize(fs_in.fNormal);

    vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
    vec3 lights = CalcDirLight(ambientLight, normal, viewDir);
    
    //for(int i = 0; i < NR_POINT_LIGHTS; i++) {
    //    lights += CalcPointLight(pointLights[i], normal, fs_in.fPos.xyz, viewDir);
    //}

    //
	//lights += CalcSpotLight(spotLight, normal, fs_in.fPos.xyz, viewDir);

    fColor = vec4(lights, 0.9f);
}

vec3 CalcDirLight(Light dirLight, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 ambient = dirLight.color * material.ambient * dirLight.ads.x;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse =  dirLight.color * material.diffuse * diff * dirLight.ads.y;

    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0);
    specular =  dirLight.color * (dirLight.ads.z * spec * material.specular);

    return vec3(0, 1, 0) ;
}

vec3 CalcPointLight(Light pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(pointLight.position - fs_in.fPos);
    vec3 ambient = pointLight.ads.x * pointLight.color * color;

    float distance = length(pointLight.position - fs_in.fPos);
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
    vec3 lightDir = normalize(spotlight.position - fs_in.fPos);
    vec3 ambient = spotlight.ads.x * spotlight.color * color;

    float theta = dot(lightDir, normalize(-spotlight.direction));
    float epsilon   = spotlight.coB.z - spotlight.coB.y;
    float intensity = clamp((theta - spotlight.coB.y) / epsilon, 0.0, 1.0);

    float distance = length(spotlight.position - fs_in.fPos);
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
