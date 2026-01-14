#version 450

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 3) out;

struct Light {
    mat4 light_space_matrix;
    mat4 projection;
    mat4 view;
    mat4 light_space_matrix_left;
    mat4 light_space_matrix_top;
    mat4 light_space_matrix_right;
    mat4 light_space_matrix_bottom;
    mat4 light_space_matrix_back;
    mat4 cascade0;

    vec4 ads;
    vec4 clq;
    vec4 coB;
    vec4 color;
    vec4 direction;
    vec4 position;

    vec4 cascade_scale1;
    vec4 cascade_scale2;
    vec4 cascade_scale3;
    vec4 cascade_offset1;
    vec4 cascade_offset2;
    vec4 cascade_offset3;

    vec4 cascade_min_splits;
    vec4 cascade_max_splits;
    vec4 cascade_texel_sizes;
};

#define NR_POINT_LIGHTS 2

layout(std430, binding = 1) readonly buffer LightBuffer {
    Light lights[];
} lightData;

layout(location = 0) out vec4 position;

void main()
{
  Light light = lightData.lights[1];

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
