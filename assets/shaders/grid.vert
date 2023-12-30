#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO 
{
    mat4 model;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[1];
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

vec3 unproject(float x, float y, float z, mat4 view, mat4 proj) {
    vec4 unprojected =  inverse(view) * inverse(proj) * vec4(x, y, z, 1.0);
    return unprojected.xyz / unprojected.w;
}

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;
layout(location = 3) out mat4 matView;
layout(location = 7) out mat4 matProj;
layout(location = 11) out float near;
layout(location = 12) out float far;

void main() {
	vec3 point = pos.xyz;
	nearPoint = unproject(point.x, point.y, 0.0, pc.view, ubos[gl_InstanceIndex].proj).xyz;
	farPoint = unproject(point.x, point.y, 1.0, pc.view, ubos[gl_InstanceIndex].proj).xyz;
	matView = pc.view;	
	matProj = ubos[gl_InstanceIndex].proj;
	near = pc.viewPos.x;
	far = pc.viewPos.y;
    gl_Position = vec4(point, 1.0);
}
