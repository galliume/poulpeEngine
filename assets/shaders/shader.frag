#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 cameraPos;
layout(location = 3) in vec3 fragModelPos;
layout(location = 4) in float fragAmbiantLight; 
layout(location = 5) in float fragFogDensity; 
layout(location = 6) in vec3 fragFogColor;
layout(location = 7) in vec3 fragLightPos;

layout(binding = 1) uniform sampler2D texSampler;

//todo move to push constants
vec3 lightColor = vec3(1.0f);
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
    float specularStrength = 0.5;  
    vec3 viewDir = normalize(cameraPos - fragModelPos);
    vec3 lightDir = normalize(fragLightPos - fragModelPos);
    vec3 norm = normalize(fragNormal);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 ambient = fragAmbiantLight * lightColor;
    vec3 diffuse = diff * lightColor * 0.05;

    outputColor = texture(texSampler, fragTexCoord);
    outputColor = vec4((ambient + diffuse), 1.0) * outputColor;
    
    if (0 < fragFogDensity) {
        float d = distance(cameraPos, fragModelPos);
        float alpha = getFogFactor(d);

        outputColor = mix(outputColor, vec4(fragFogColor, 1.0), alpha * fragFogDensity);
    }

    //outputColor = vec4(fragColor, 1.0);
}