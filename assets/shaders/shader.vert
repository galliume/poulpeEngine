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
    float shininess;
    mat4 view;
    vec4 viewPos;
    vec3 mapsUsed;
    vec3 ambient;
    vec3 ambientLight;
    vec3 ambientLightColor;
    vec3 lightDir;
    vec3 diffuseLight;
    vec3 specular;
    vec3 specularLight;
} pc;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;

layout(location = 0) out VS_OUT {
    flat int fTextureID;
    float fShininess;
    float fConstant;
    float fLinear;
    float fQuadratic;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec3 fMapsUsed;
    vec3 fAmbient;
    vec3 fAmbientLight;
    vec3 fAmbientLightColor;
    vec3 fLightDir;
    vec3 fDiffuseLight;
    vec3 fSpecular;
    vec3 fSpecularLight;
    vec4 fViewPos;
    mat3 TBN;
} vs_out;

void main()
{
    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

    vs_out.fNormal = normal;
    //fNormal = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
    vs_out.fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    vs_out.fTexCoord = texCoord;
    vs_out.fTextureID = pc.textureID;
    vs_out.fMapsUsed = pc.mapsUsed;
    vs_out.fViewPos = pc.viewPos;
    vs_out.fAmbient = pc.ambient;
    vs_out.fAmbientLight = pc.ambientLight;
    vs_out.fAmbientLightColor = pc.ambientLightColor;
    vs_out.fLightDir = pc.lightDir;
    vs_out.fAmbientLight = pc.ambientLight;
    vs_out.fDiffuseLight = pc.diffuseLight;
    vs_out.fSpecular = pc.specular;
    vs_out.fSpecularLight = pc.specularLight;
    vs_out.fShininess = pc.shininess;
    vs_out.fConstant = ubos[gl_InstanceIndex].constant;
    vs_out.fLinear = ubos[gl_InstanceIndex].linear;
    vs_out.fQuadratic = ubos[gl_InstanceIndex].quadratic;

    vec3 t = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(tangent.xyz, 0.0)));
    vec3 n = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(normal, 0.0)));
    vec3 b = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(cross(n, t), 0.0)));
    mat3 TBN = transpose(mat3(t, b, n));
    vs_out.TBN = TBN;
} 