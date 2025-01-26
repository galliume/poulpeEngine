#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define GAMMA_TRESHOLD 0.0031308

#define HEIGHT_MAP 0
#define TERRAIN_GROUND 1
#define TERRAIN_GRASS 2
#define TERRAIN_SNOW 3
#define TERRAIN_SAND 4

layout(location = 0) out vec4 final_color;

layout(location = 0) in vec2 in_texture_coord;
layout(location = 1) in vec4 in_weights;

layout(binding = 1) uniform sampler2D tex_sampler[5];

float linear_to_sRGB(float color)
{
  if (color <= GAMMA_TRESHOLD) {
    return color * 12.92;
  } else {
    return pow((color * 1.055), 1/2.4) - 0.055;
  }
}

vec3 tex_color(int index)
{
  vec3 color = texture(tex_sampler[index], in_texture_coord).rgb;
  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);
  
  return color;
}

void main()
{
  vec3 color = vec3(0.0);

  color += tex_color(TERRAIN_SAND) * in_weights.x;
  color += tex_color(TERRAIN_GROUND) * in_weights.y;
  color += tex_color(TERRAIN_GRASS) * in_weights.z;
  color += tex_color(TERRAIN_SNOW) * in_weights.w;

  final_color = vec4(color, 1.0);
}
