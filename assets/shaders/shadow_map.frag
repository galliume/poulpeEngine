#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 light_pos;
layout(location = 1) in float far_plane;
layout(location = 2) in vec4 position;

void main()
{
  float light_distance = length(position.xyz - light_pos);
  light_distance = light_distance / far_plane;
  gl_FragDepth = light_distance;
}
