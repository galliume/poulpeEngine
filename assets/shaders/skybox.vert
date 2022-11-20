#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 fragTexCoord;
layout(location = 1) out vec4 fragCameraPos;
layout(location = 2) out vec4 fragModelPos;
layout(location = 3) out float fragAmbiantLight;
layout(location = 4) out float fragFogDensity;
layout(location = 5) out vec3 fragFogColor;

layout(push_constant) uniform constants
{
    vec4 data;
    vec4 cameraPos;
    vec4 fogColor;
    vec4 lightPos;
    mat4 view;	
} PC;

void main() 
{
    fragTexCoord = pos.xyz;
    //gl_Position = vec4(pos.xyz, 1.0);
    vec4 p = ubo.proj * PC.view * vec4(pos, 1.0);
    gl_Position = p.xyww;
    fragAmbiantLight = PC.data.y;
    fragFogDensity = PC.data.z;
    fragModelPos = gl_Position;
    fragCameraPos = ubo.proj * PC.view * ubo.model * PC.cameraPos;
    fragFogColor = PC.fogColor.xyz;
}
