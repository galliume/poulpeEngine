#version 450

layout(location = 0) out vec4 outputColor;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() {    
    outputColor = texture(texSampler, fragTexCoord);    
}