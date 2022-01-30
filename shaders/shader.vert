#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 fragColor;

void main() {
    gl_Position = vec4(inPosition + vec3(0, 0, 0.5), 1.0);
    fragColor = vec4(inColor * 0.5 + vec3(0.5), 1.0);
}