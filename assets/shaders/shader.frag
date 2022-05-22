#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec4 fogDensity;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outputColor = texture(texSampler, fragTexCoord);
    outputColor = mix(outputColor, vec4(50/255.0f, 50/255.0f, 50/255.0f, 1.0f), fogDensity.x);
    //outputColor = vec4(fragColor, 1.0);
}