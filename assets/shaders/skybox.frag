#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec3 fragTexCoord;

layout(binding = 1) uniform samplerCube texSampler;

void main() { 
    outputColor = texture(texSampler, fragTexCoord);
    //outputColor = vec4(fragTexCoord, 1.0);
}