#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo[];

layout(push_constant) uniform PER_OBJECT
{
	int imgIdx;
}pc;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out int index;


void main() {
    gl_Position = ubo[pc.imgIdx].proj * ubo[pc.imgIdx].view * ubo[pc.imgIdx].model * vec4(pos, 1.0);
    fragColor = vec4(color, 1.0);
    fragTexCoord = texCoord;
    index = pc.imgIdx;
}