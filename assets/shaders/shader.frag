#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec4 cameraPos;
layout(location = 3) in vec4 fragModelPos;
layout(location = 4) in float fragAmbiantLight; 

layout(binding = 1) uniform sampler2D texSampler;

float getFogFactor(float d)
{
    const float FogMax = 2;
    const float FogMin = 1.0;

    if (d>=FogMax) return 1;
    if (d<=FogMin) return 0;

    return 1 - (FogMax - d) / (FogMax - FogMin);
}

vec3 lightColor = vec3(1.0f);
vec4 fogColor = vec4(50/255.0f, 50/255.0f, 50/255.0f, 1.0f);

void main() {

    outputColor = texture(texSampler, fragTexCoord);

    vec3 ambient = fragAmbiantLight * lightColor;

    outputColor = vec4(ambient, 1.0f) * outputColor;

    float d = distance(cameraPos, fragModelPos);
    float alpha = getFogFactor(d);

    //outputColor = mix(outputColor, fogColor, alpha);
    //outputColor = vec4(fragColor, 1.0);
}