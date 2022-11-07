#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct ObjectData 
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(std140,set = 0, binding = 0) readonly buffer ObjectBuffer{

	ObjectData data[];
} objectBuffer;


layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragCameraPos;
layout(location = 3) out vec3 fragModelPos;
layout(location = 4) out float fragAmbiantLight;
layout(location = 5) out float fragFogDensity;
layout(location = 6) out vec3 fragFogColor;
layout(location = 7) out vec3 fragLightPos;
layout(location = 8) out int fragTextureID;

layout(push_constant) uniform constants
{
    int textureID;
    vec4 cameraPos;
    float ambiantLight;
    float fogDensity;
    vec3 fogColor;
    vec4 lightPos;
	
} PC;

void main() {
    
    gl_Position = objectBuffer.data[gl_BaseInstance].proj * objectBuffer.data[gl_BaseInstance].view * objectBuffer.data[gl_BaseInstance].model * vec4(pos, 1.0);

    fragTexCoord = texCoord;
    fragNormal =  mat3(transpose(inverse(objectBuffer.data[gl_BaseInstance].model))) * normal;
    fragModelPos = vec3(objectBuffer.data[gl_BaseInstance].model * vec4(pos, 1.0));
    fragCameraPos = PC.cameraPos.xyz;
    fragAmbiantLight = PC.ambiantLight;
    fragFogDensity = PC.fogDensity;
    fragFogColor = PC.fogColor;
    fragLightPos = PC.lightPos.xyz;
    fragTextureID = PC.textureID;
}
