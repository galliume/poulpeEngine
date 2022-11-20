#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO 
{
    mat4 model;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[341];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant) uniform constants
{
    vec4 data;
    vec4 cameraPos;
    vec4 fogColor;
    vec4 lightPos;
    mat4 view;	
} PC;

void main() {
    gl_Position = ubos[gl_InstanceIndex].proj * PC.view * ubos[gl_InstanceIndex].model * vec4(pos, 1);
	fragColor = normal;
    fragTexCoord = texCoord;
}