#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

#define GAMMA_TRESHOLD 0.0031308

#define DIFFUSE_INDEX 0
#define DEPTH_INDEX 1

layout(location = 0) out vec4 final_color;

layout(location = 0) in vec2 in_texture_coord;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_position;
layout(location = 3) in vec3 in_view_position;
layout(location = 4) in mat3 in_inverse_model;

layout(binding = 1) uniform sampler2D tex_sampler[2];

float linear_to_sRGB(float color)
{
  if (color <= GAMMA_TRESHOLD) {
    return color * 12.92;
  } else {
    return pow((color * 1.055), 1/2.4) - 0.055;
  }
}

vec3 linear_to_hdr10(vec3 color, float white_point)
{
  // Convert Rec.709 to Rec.2020 color space to broaden the palette
  const mat3 from709to2020 = mat3(
      0.6274040, 0.3292820, 0.0433136,
      0.0690970, 0.9195400, 0.0113612,
      0.0163916, 0.0880132, 0.8955950
  );
  //color = from709to2020 * color;

  // Normalize HDR scene values ([0..>1] to [0..1]) for ST.2084 curve
  const float st2084_max = 10000.0;
  color *= white_point / st2084_max;

  // Apply ST.2084 (PQ curve) for HDR10 standard
  const float m1 = 2610.0 / 4096.0 / 4.0;
  const float m2 = 2523.0 / 4096.0 * 128.0;
  const float c1 = 3424.0 / 4096.0;
  const float c2 = 2413.0 / 4096.0 * 32.0;
  const float c3 = 2392.0 / 4096.0 * 32.0;
  vec3 cp = pow(abs(color), vec3(m1));
  color = pow((c1 + c2 * cp) / (1.0 + c3 * cp), vec3(m2));

  return color;
}

vec3 srgb_to_linear(vec3 color)
{ 
  float gamma        = 2.4f; // The sRGB curve for mid tones to high lights resembles a gamma of 2.4
  vec3 linear_low  = color / 12.92;
  vec3 linear_high = pow((color + 0.055) / 1.055, vec3(gamma));
  vec3 is_high     = step(0.0404482362771082, color);
  return mix(linear_low, linear_high, is_high);
}

vec3 tex_color(int index)
{
  vec3 color = texture(tex_sampler[index], in_texture_coord).rgb;
  color.r = linear_to_sRGB(color.r);
  color.g = linear_to_sRGB(color.g);
  color.b = linear_to_sRGB(color.b);

  return color;
}

float LinearizeDepth(float depth) 
{
  float near = 0.1;
  float far = 1000.0;

  return near * far / (far - depth * (far - near));
}

void main()
{
  //@todo do PBR for water ?
  vec3 water_color = vec3(0.f,105.f/255.f,148.f/255.f);
  //color =  tex_color(DIFFUSE_INDEX);

  float water_depth = LinearizeDepth(gl_FragCoord.z);
  float terrain_depth = LinearizeDepth(texture(tex_sampler[DEPTH_INDEX], in_texture_coord).r);
  float depth = abs(terrain_depth - water_depth);
  float falloff_distance = 0.1;
  float edge_sharpness  = 0.005;

  float foam_factor = smoothstep(falloff_distance, falloff_distance * edge_sharpness, depth);
  vec3 foam_color = vec3(1.0, 0.0, 0.0);

  vec3 color = vec3(water_depth);//mix(water_color, foam_color, foam_factor);

  vec3 light_color = vec3(1.0);
  vec3 ambient = 0.1 * light_color;

  //@todo a point lights...

  //sun directionnal light
  vec3 light_pos = vec3(150.0, 10.0, 0.0);

  vec3 norm = normalize(in_normal);
//  vec3 x = dFdx(in_position);
//  vec3 y = dFdy(in_position);
//  vec3 norm = in_inverse_model * normalize(cross(x, y));
//
  vec3 light_dir = normalize(-light_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = diff * light_color;

  vec3 view_dir = normalize(in_view_position - in_position);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 2);
  vec3 specular = 0.5 * spec * light_color;

  vec3 result = (ambient + diffuse + specular) * color;

  float exposure = 10.0;
  result.rgb = vec3(1.0) - exp(-result.rgb* exposure);

  //@todo check how to get the precise value
  float white_point = 350;
  final_color = vec4(0.0, 0.0, 0.0, 0.2);
  final_color.rgb = linear_to_hdr10(result.rgb, white_point);

//  result.r = linear_to_sRGB(result.r);
//  result.g = linear_to_sRGB(result.g);
//  result.b = linear_to_sRGB(result.b);

  final_color.rgb = result;
}
