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

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 fidtidBB;
layout(location = 5) in vec3 vColor;

layout(push_constant) uniform constants
{
    //texture ID blank blank
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
    vec3 mapsUsed;
} pc;

layout(location = 0) out VS_OUT {
    vec2 fTexCoord;
} vs_out;

void main()
{
    vs_out.fTexCoord = texCoord;
    gl_Position = vec4(pos, 1.0);
}
