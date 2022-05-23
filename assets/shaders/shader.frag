#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec4 cameraPos;
layout(location = 3) in vec4 fragModelPos;
layout(location = 4) in float fragAmbiantLight; 
layout(location = 5) in float fragFogDensity; 

layout(binding = 1) uniform sampler2D texSampler;

vec3 lightColor = vec3(1.0f);
vec4 fogColor = vec4(50/255.0f, 50/255.0f, 50/255.0f, 1.0f);

const float fogDepthMax = 2;
const float fogDepthMin = 1.0;
const float fogDistanceMax = 2;
const float fogDistanceMin = 1.0;

void main() {

    outputColor = texture(texSampler, fragTexCoord);

    //float alpha = getFogFactor(d);
    vec3 ambient = fragAmbiantLight * lightColor;
    outputColor = vec4(ambient, 1.0f) * outputColor;
    
    if (0 < fragFogDensity) {
        vec4 depthVert = cameraPos * fragModelPos;
        float depth = abs(depthVert.z / depthVert.w);
        float fogDensity = 1.0 - clamp((fogDepthMax - depth) / (fogDepthMin - fogDepthMax), 0.0, 1.0);
        float d = distance(cameraPos, fragModelPos);
        if (d>=fogDistanceMax) fogDensity = 1;
        if (d<=fogDistanceMin) fogDensity = 0;

        outputColor = mix(outputColor, fogColor, fogDensity  * fragFogDensity);
    }

    //outputColor = vec4(fragColor, 1.0);
}