#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout(location = 0) in FRAG_VAR {
  float far_plane;
  vec4 frag_position;
  vec3 light_position;
  mat4 light_space_matrix;
  mat4 light_space_matrix_left;
  mat4 light_space_matrix_top;
  mat4 light_space_matrix_right;
  mat4 light_space_matrix_bottom;
  mat4 light_space_matrix_back;
} var[];

layout(location = 0) out vec3 light_pos;
layout(location = 1) out float far_plane;
layout(location = 2) out vec4 position;


void main()
{
  for(int face = 0; face < 6; ++face)
  {
    gl_Layer = face;
    for(int i = 0; i < 3; ++i)
    {
      //@todo fix ?
      if (i == 0) {
        gl_Position = var[i].light_space_matrix * var[i].frag_position;
      } else if (i == 1) {
        gl_Position = var[i].light_space_matrix_left * var[i].frag_position;
      } else if (i == 2) {
        gl_Position = var[i].light_space_matrix_top * var[i].frag_position;
      } else if (i == 3) {
        gl_Position = var[i].light_space_matrix_bottom * var[i].frag_position;
      } else if (i == 4) {
        gl_Position = var[i].light_space_matrix_right * var[i].frag_position;
      } else if (i == 5) {
        gl_Position = var[i].light_space_matrix_back * var[i].frag_position;
      }
      position = gl_Position;
      light_pos = var[i].light_position;
      far_plane = var[i].far_plane;
      EmitVertex();
    }
    EndPrimitive();
  }
}
