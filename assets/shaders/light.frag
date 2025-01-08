#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vertex_color;

layout(location = 0) out vec4 final_color;

void main()
{
  final_color = vec4(vertex_color, 1.0);
}
