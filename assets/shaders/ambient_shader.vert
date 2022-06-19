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
   UBO ubos[341];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragCameraPos;
layout(location = 3) out vec4 fragModelPos;
layout(location = 4) out float fragAmbiantLight;
layout(location = 5) out float fragFogDensity;
layout(location = 6) out vec3 fragFogColor;
layout(location = 7) out vec4 fragLightPos;

layout(push_constant) uniform constants
{
    int textureID;
    vec4 cameraPos;
    float ambiantLight;
    float fogDensity;
    vec3 fogColor;
    vec4 lightPos;
} PushConstants;

void main() {
    
    gl_Position = ubos[gl_InstanceIndex].proj * ubos[gl_InstanceIndex].view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);
    fragTexCoord = texCoord;
    fragNormal =  mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
    fragModelPos = gl_Position;
    fragCameraPos = ubos[gl_InstanceIndex].proj * ubos[gl_InstanceIndex].view * ubos[gl_InstanceIndex].model * PushConstants.cameraPos;
    fragAmbiantLight = PushConstants.ambiantLight;
    fragFogDensity = PushConstants.fogDensity;
    fragFogColor = PushConstants.fogColor;
    fragLightPos = PushConstants.lightPos;
}
