#version 450
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(set = 0, binding = 0) uniform ShadowMatrices {
    mat4 shadowMatrices[6];
};

layout(location = 0)out vec4 FragPos; // Fragment position passed to the fragment shader

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // Specify the face of the cube map
        for (int i = 0; i < 3; ++i) // Iterate over the vertices of the triangle
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}