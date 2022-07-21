#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragModelPos;

layout(binding = 1) uniform sampler2D texSampler[];

layout(push_constant) uniform constants
{
    int textureID;
    vec4 cameraPos;
    float ambiantLight;
    float fogDensity;
    vec3 fogColor;
    vec4 lightPos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} PC;

//todo move to push constants
vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
const float fogDistanceMax = 1.0;
const float fogDistanceMin = 0.5;

float getFogFactor(float d)
{
    //todo move to push constants
    const float FogMax = 1.5;
    const float FogMin = 0.5;

    if (d>=FogMax) return 1;
    if (d<=FogMin) return 0;

    return 1 - (FogMax - d) / (FogMax - FogMin);
}

void main() {
  
    vec3 viewDir = normalize(PC.cameraPos.xyz - fragModelPos);
    vec3 lightDir = normalize(PC.lightPos.xyz - fragModelPos);
    vec3 norm = normalize(fragNormal);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), PC.shininess);
    vec3 specular = spec * PC.specular * lightColor;

    vec3 ambient = PC.ambiantLight * lightColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * PC.diffuse;

    outputColor = texture(texSampler[PC.textureID], fragTexCoord);
    outputColor.xyz = outputColor.xyz * (ambient + diffuse + specular);
    //outputColor.xyz = outputColor.xyz * diffuse;
    //outputColor.xyz = outputColor.xyz * specular;
    if (0 < PC.fogDensity) {
        float d = distance(PC.cameraPos.xyz, fragModelPos);
        float alpha = getFogFactor(d);

        outputColor = mix(outputColor, vec4(PC.fogColor, 1.0), alpha * PC.fogDensity);
    }

    //outputColor = vec4(fragColor, 1.0);
}