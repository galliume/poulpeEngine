#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_UBOS 1000

struct UBO 
{
    mat4 model;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out int fragTextureID;
layout(location = 2) out vec3 fragCameraPos;
layout(location = 3) out vec3 fragModelPos;
layout(location = 4) out float fragAmbiantLight;
layout(location = 5) out float fragFogDensity;
layout(location = 6) out vec3 fragFogColor;
layout(location = 7) out vec3 fragLightPos;
layout(location = 8) out vec3 fragNormal;

layout(push_constant) uniform constants
{
    vec4 data;
    vec4 cameraPos;
    vec3 fogColor;
    vec4 lightPos;
    mat4 view;
	float ambiantLight;
	float fogDensity;
} PC;

void main() {
    
    gl_Position = ubos[gl_InstanceIndex].proj * PC.view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

    fragTexCoord = texCoord;
    fragTextureID = int(PC.data.x);
	fragAmbiantLight = PC.ambiantLight;
    fragFogDensity = PC.fogDensity;
    fragFogColor = PC.fogColor;
    fragLightPos = PC.lightPos.xyz;
	fragModelPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    fragCameraPos = PC.cameraPos.xyz;
	fragNormal =  mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
}
