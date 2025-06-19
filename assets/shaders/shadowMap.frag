#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in FRAG_VAR {
  vec3 light_pos;
  float far_plane;
  vec4 position;
} var;

void main()
{
  float light_distance = length(var.position.xyz - var.light_pos);
  light_distance = light_distance / var.far_plane;
  gl_FragDepth = light_distance;
}