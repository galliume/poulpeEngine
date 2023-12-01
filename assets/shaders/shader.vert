#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_UBOS 1000

struct UBO
{
    mat4 model;
    mat4 projection;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(push_constant) uniform constants
{
    int textureID;
    mat4 view;
    vec4 viewPos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} pc;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fNormal;
layout(location = 1) out vec3 fPos;
layout(location = 2) out vec2 fTexCoord;
layout(location = 3) flat out int fTextureID;
layout(location = 4) out vec4 fViewPos;
layout(location = 5) out vec3 fAmbient;
layout(location = 6) out vec3 fDiffuse;
layout(location = 7) out vec3 fSpecular;
layout(location = 8) out float fShininess;

void main()
{
    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

    fNormal = normal;
    //fNormal = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
    fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    fTexCoord = texCoord;
    fTextureID = pc.textureID;
    fViewPos = pc.viewPos;
    fAmbient = pc.ambient;
    fDiffuse = pc.diffuse;
    fSpecular = pc.specular;
    fShininess = pc.shininess;
} 