#version 450

layout(location = 0) out vec4 fColor;

layout(location = 0) in vec3 fPos;
layout(location = 1) in vec3 fNormal;

void main()
{
    vec3 normalColor = normalize(fNormal) * 0.5 + 0.5;

    fColor = vec4(normalColor, 1.0);
}  