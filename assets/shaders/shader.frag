#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec4 cameraPos;
layout(location = 3) in vec4 fragModelPos;

layout(binding = 1) uniform sampler2D texSampler;


float getFogFactor(float d)
{
    const float FogMax = 2.5;
    const float FogMin = 2.0;

    if (d>=FogMax) return 1;
    if (d<=FogMin) return 0;

    return 1 - (FogMax - d) / (FogMax - FogMin);
}


void main() {

    float d = distance(cameraPos, fragModelPos);
    float alpha = getFogFactor(d);

    outputColor = texture(texSampler, fragTexCoord);
    outputColor = mix(outputColor, vec4(50/255.0f, 50/255.0f, 50/255.0f, 1.0f), alpha);
    //outputColor = vec4(fragColor, 1.0);
}