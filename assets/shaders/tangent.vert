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
    //texture ID blank blank
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
} pc;

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec2 texture_coord;
//layout(location = 6) in ivec4 bone_ids;
//layout(location = 7) in vec4 bone_weights;
//layout(location = 8) in float total_weight;

layout(location = 0) out vec3 fPos;
layout(location = 1) out vec3 fNormal;

void main()
{
    fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    fNormal = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * tangent.xyz;

    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * ubos[gl_InstanceIndex].model * vec4(fPos, 1.0);
} 