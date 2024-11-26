#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
}  