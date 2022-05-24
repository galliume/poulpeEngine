#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec3 fragTexCoord;
layout(location = 1) in vec4 cameraPos;
layout(location = 2) in vec4 fragModelPos;
layout(location = 3) in float fragAmbiantLight; 
layout(location = 4) in float fragFogDensity; 

layout(binding = 1) uniform samplerCube texSampler;

//todo move to push constants
vec3 lightColor = vec3(1.0f);
vec4 fogColor = vec4(25/255.0f, 25/255.0f, 25/255.0f, 1.0f);
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

void main() 
{ 
    outputColor = texture(texSampler, fragTexCoord);

    vec3 ambient = fragAmbiantLight * lightColor;
    outputColor = vec4(ambient, 1.0f) * outputColor;

    if (0 < fragFogDensity) {
        float d = distance(cameraPos, fragModelPos);
        float alpha = getFogFactor(d);

        outputColor = mix(outputColor, fogColor, alpha * fragFogDensity);
    }

    //outputColor = vec4(fragTexCoord, 1.0);
    //outputColor = vec4(1.0, 0.0, 0.0, 1.0);
}