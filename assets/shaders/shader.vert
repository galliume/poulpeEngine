#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_UBOS 1000

struct UBO
{
    mat4 model;
    mat4 projection;
    float constant;
    float linear;
    float quadratic;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(push_constant) uniform constants
{
    int textureID;
    int mapsUsed;
    float shininess;
    mat4 view;
    vec4 viewPos;
    vec3 ambient;
    vec3 ambientLight;
    vec3 lightDir;
    vec3 diffuseLight;
    vec3 specular;
    vec3 specularLight;
} pc;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;

layout(location = 0) out vec3 fNormal;
layout(location = 1) out vec3 fPos;
layout(location = 2) out vec2 fTexCoord;
layout(location = 3) flat out int fTextureID;
layout(location = 4) out vec4 fViewPos;
layout(location = 5) out vec3 fAmbient;
layout(location = 6) out vec3 fAmbientLight;
layout(location = 7) out vec3 fLightDir;
layout(location = 8) out vec3 fDiffuseLight;
layout(location = 9) out vec3 fSpecular;
layout(location = 10) out vec3 fSpecularLight;
layout(location = 11) out float fShininess;
layout(location = 12) out int fMapsUsed;
layout(location = 13) out float fConstant;
layout(location = 14) out float fLinear;
layout(location = 15) out float fQuadratic;

layout(location = 16) out VS_OUT {
//    vec3 fPos;
//    vec2 fTexCoord;
    mat3 TBN;
} vs_out;

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
    fAmbientLight = pc.ambientLight;
    fLightDir = pc.lightDir;
    fAmbientLight = pc.ambientLight;
    fDiffuseLight = pc.diffuseLight;
    fSpecular = pc.specular;
    fSpecularLight = pc.specularLight;
    fShininess = pc.shininess;
    fMapsUsed = pc.mapsUsed;
    fConstant = ubos[gl_InstanceIndex].constant;
    fLinear = ubos[gl_InstanceIndex].linear;
    fQuadratic = ubos[gl_InstanceIndex].quadratic;

    vec3 t = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(tangent.xyz, 0.0)));
    vec3 n = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(normal, 0.0)));
    vec3 b = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(cross(n, t), 0.0)));
    mat3 TBN = transpose(mat3(t, b, n));
    vs_out.TBN = TBN;
} 