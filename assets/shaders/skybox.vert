#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(std140, binding = 0) readonly uniform UniformBufferObject {
    mat4 model;
    mat4 projection;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 color;

layout(location = 0) out vec3 fTexCoord;
layout(location = 1) out vec4 fCameraPos;
layout(location = 2) out vec4 fModelPos;

layout(std140, push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
  vec4 options;
} pc;

void main()
{
  fTexCoord = position;
  //gl_Position = vec4(pos.xyz, 1.0);
  vec4 p = ubo.projection * pc.view * vec4(position, 1.0);
  gl_Position = p.xyww;
  fModelPos = gl_Position;
  fCameraPos = ubo.projection * pc.view * ubo.model * vec4(pc.view_position, 1.0);
}
