#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() {    
    outputColor = texture(texSampler, fragTexCoord);    
}