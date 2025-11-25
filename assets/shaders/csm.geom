#version 450

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;

struct Light {
  mat4 light_space_matrix;
  mat4 projection;
  mat4 view;
  vec3 ads;
  vec3 clq;
  vec3 coB;
  vec3 color;
  vec3 direction;
  vec3 position;
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
  mat4 cascade_scale_offset;
  mat4 cascade_scale_offset1;
  mat4 cascade_scale_offset2;
  mat4 cascade_scale_offset3;
  vec4 cascade_min_splits;
  vec4 cascade_max_splits;
  vec4 cascade_texel_size;
};

#define NR_POINT_LIGHTS 2

layout(set = 0, binding = 1) readonly buffer ObjectBuffer {
  Light sun_light;
  Light point_lights[NR_POINT_LIGHTS];
  Light spot_light;
};

layout(location = 0) out vec4 position;

void main()
{
  Light light = sun_light;
  mat4 light_matrices[4] = mat4[4](
    light.light_space_matrix,          // slice 0
    light.light_space_matrix_left,     // slice 1
    light.light_space_matrix_top,      // slice 2
    light.light_space_matrix_right     // slice 3
  );

  for (int i = 0; i < 3; ++i)
  {
    position = gl_in[i].gl_Position;
    gl_Position = light_matrices[gl_InvocationID] * gl_in[i].gl_Position;
    gl_Layer = gl_InvocationID;
    EmitVertex();
  }
  EndPrimitive();
}
