#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec3 fragTexCoord;
layout(location = 1) in vec3 fragColor;

layout(binding = 1) uniform samplerCube texSampler;

void main() 
{ 
    //outputColor = texture(texSampler, fragTexCoord);
    //outputColor = vec4(fragColor, 1.0);
    outputColor = vec4(1.0, 0.0, 0.0, 1.0);
}