#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 fragTexCoord;

void main() 
{
    fragTexCoord = pos.xyz;
    //gl_Position = vec4(pos.xyz, 1.0);
    vec4 p = ubo.proj * ubo.view * vec4(pos, 1.0);
    gl_Position = p.xyww;
}