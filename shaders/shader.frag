#version 450

layout(location = 0) out vec4 outputColor;

layout(location = 0) in vec4 fragColor;

void main() {
    
    outputColor  = fragColor;
}