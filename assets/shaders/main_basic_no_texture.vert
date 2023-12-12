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
    //texture ID blank blank
    vec3 textureIDBB;
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
    vec2 fTexCoord;
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
    vs_out.fTextureID = int(pc.textureIDBB.x);
    vs_out.fMapsUsed = pc.mapsUsed;
    vs_out.fViewPos = pc.viewPos;
} 