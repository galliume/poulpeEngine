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

layout(location = 0) out vec2 outUV;

void main()
{
    gl_Position = ubos[gl_InstanceIndex].projection * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

	//outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	//gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
} 