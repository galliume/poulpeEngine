#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
    int index;
}; 

layout(set = 0, binding = 0) uniform CubeUniformBufferObject {
   UBO ubo;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragTexCoord;

void main() { 
     //gl_Position = vec4(pos, 1.0);
    gl_Position = ubo.proj * ubo.model * vec4(pos.xyz, 1.0);
    fragTexCoord = pos;
}