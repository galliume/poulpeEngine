#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 final_color;

layout(location = 0) in vec2 in_tex_coords;
layout(location = 1) in vec3 in_color;

layout(binding = 1) uniform sampler2D tex_sampler[1];

void main()
{
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex_sampler[0], in_tex_coords).r);
  final_color = vec4(1.0, 1.0, 1.0, 1.0) * sampled;
}
