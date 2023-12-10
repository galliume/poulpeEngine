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
    vec2 texSize;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(push_constant) uniform constants
{
    int textureID;
    mat4 view;
    vec4 viewPos;
    vec3 mapsUsed;
} pc;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;

layout(location = 0) out VS_OUT {
    flat int fTextureID;
    float fConstant;
    float fLinear;
    float fQuadratic;
    vec2 fTexCoord;
    vec2 fTexSize;
    vec3 fNormal;
    vec3 fPos;
    vec3 fMapsUsed;
    vec4 fViewPos;
    mat3 TBN;
} vs_out;

void main()
{
    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

    
    vec3 t = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(tangent.xyz, 0.0)));
    vec3 n = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(normal, 0.0)));
    vec3 b = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(cross(n, t) * tangent.w, 0.0)));
    mat3 TBN = transpose(mat3(t, b, n));
    vs_out.TBN = TBN;

    vs_out.fNormal = normal;
    //fNormal = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
    vs_out.fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    vs_out.fTexCoord = texCoord;
    vs_out.fTextureID = pc.textureID;
    vs_out.fMapsUsed = pc.mapsUsed;
    vs_out.fViewPos = pc.viewPos;
    vs_out.fConstant = ubos[gl_InstanceIndex].constant;
    vs_out.fLinear = ubos[gl_InstanceIndex].linear;
    vs_out.fQuadratic = ubos[gl_InstanceIndex].quadratic;
    vs_out.fTexSize = ubos[gl_InstanceIndex].texSize;
} 