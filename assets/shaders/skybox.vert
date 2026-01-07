#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(std140, binding = 0) readonly uniform UniformBufferObject {
    mat4 model;
    mat4 projection;
} ubo;

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 original_pos;
layout(location = 5) in vec2 texture_coord;
layout(location = 6) in ivec4 bone_ids;
layout(location = 7) in vec4 bone_weights;
layout(location = 8) in float total_weight;

layout(location = 0) out vec3 tex_coord;
layout(location = 1) out vec3 view_pos;

layout(std140, push_constant) uniform constants
{
  mat4 view;
  vec4 view_position;
  layout(offset = 80) uint env_options;
  layout(offset = 96) uint options;
} pc;

void main()
{
  tex_coord = position;
  //gl_Position = vec4(pos.xyz, 1.0);
  vec4 p = ubo.projection * pc.view * vec4(position, 1.0);
  gl_Position = p.xyww;
  view_pos = vec3(pc.view_position) - gl_Position.xyz;
}
