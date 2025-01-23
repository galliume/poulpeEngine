#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 final_color;
layout(location = 0) in vec2 coord;
layout(location = 1) in vec3 pos;

layout(binding = 1) uniform sampler2D tex_sampler[1];

void main()
{
  float height = texture(tex_sampler[0], coord).r;

  vec3 color = vec3(height);

  if (height <= 0.1) {
    color = vec3(0.1, 0.1, 1.0);
  } else if (height > 0.1 && height <= 0.4) {
    color = vec3(0.1, 1.0, 0.0);
  } else if (height > 0.4 && height <= 0.8) {
    color = vec3(0.4, 0.2, 0.0);
  }

  final_color = vec4(color, 1.0);
}
