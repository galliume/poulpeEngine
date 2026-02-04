#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 final_color;

layout(location = 0) in vec2 in_tex_coords;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_options;

layout(binding = 1) uniform sampler2D tex_sampler[1];

void main()
{
  vec4 base_color = vec4(in_color, 1.0);
  float dist_alpha_mask = texture(tex_sampler[0], in_tex_coords).r;

  vec4 outline_color = vec4(0.1, 0.1, 0.2, 1.0);
  float outline_min_value0 = 0.5;
  float outline_min_value1 = 0.5;
  float outline_max_value0 = 0.55;
  float outline_max_value1 = 0.55;
  float outline_factor = 1.0;

  if (in_options.x > 0.5) {
    if(dist_alpha_mask >= outline_min_value0 && dist_alpha_mask <= outline_max_value1) {
      if (dist_alpha_mask <= outline_min_value1) {
        outline_factor = smoothstep(outline_min_value0, outline_min_value1, dist_alpha_mask);
      } else {
        outline_factor = smoothstep(outline_max_value1, outline_max_value0, dist_alpha_mask);
      }
      base_color = mix(outline_color, base_color, outline_factor);
    }
  }

  float soft_edge_min = 0.55;
  float soft_edge_max = 0.6;

  if (in_options.y > 0.5) {
    base_color.a *= smoothstep(soft_edge_min, soft_edge_max, dist_alpha_mask);
  } else if (dist_alpha_mask < 0.5) {
    base_color.a = 0.0;
  }

  if (in_options.z > 0.5) {
    float outer_glow_min_dvalue = 0.4;
    float outer_glow_max_dvalue = 0.5;

    if (dist_alpha_mask >= outer_glow_min_dvalue && dist_alpha_mask <= outer_glow_max_dvalue) {
      vec4 outer_glow_color = vec4(1.0, 0.0, 0.0, 0.5)
           * smoothstep(outer_glow_min_dvalue, outer_glow_max_dvalue, dist_alpha_mask);
      base_color = mix(outer_glow_color, base_color, dist_alpha_mask);
    }
  }

  final_color = base_color;
}