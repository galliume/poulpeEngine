#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[10000];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;


void main() {
    gl_Position = ubos[gl_InstanceIndex].proj * ubos[gl_InstanceIndex].view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);
    fragTexCoord = texCoord;
}