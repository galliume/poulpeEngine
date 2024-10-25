#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
    mat4 model;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 fidtidBB;
layout(location = 5) in vec3 vColor;

layout(location = 0) out vec3 fTexCoord;
layout(location = 1) out vec4 fCameraPos;
layout(location = 2) out vec4 fModelPos;

layout(push_constant) uniform constants
{
    //texture ID blank blank
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
} pc;

void main() 
{
    fTexCoord = pos.xyz;
    //gl_Position = vec4(pos.xyz, 1.0);
    vec4 p = ubo.projection * pc.view * vec4(pos, 1.0);
    gl_Position = p.xyww;
    fModelPos = gl_Position;
    fCameraPos = ubo.projection * pc.view * ubo.model * pc.viewPos;

}
