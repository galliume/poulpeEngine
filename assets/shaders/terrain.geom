#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 8) out;

layout(location = 0) in GEOM_VAR {
  vec3 normal;
	mat4 view;
} var[];

const float normal_length = 0.005;
 
struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
  UBO ubo;
};

layout (location = 0) out vec3 vertex_color;

void main()
{
/*
	for(int i = 0; i < gl_in.length(); i++) {
		vec4 pos = gl_in[i].gl_Position;
		vec4 normal = vec4(var[i].normal, 1.0);
			
		gl_Position = ubo.projection * pos;
		vertex_color = vec3(1.0, 1.0, 0.0);
		EmitVertex();
		
		gl_Position = ubo.projection * (pos + normal * normal_length);
		EmitVertex();
		vertex_color = vec3(1.0, 1.0, 0.0);

		EndPrimitive();
	}
*/
  vec3 P0 = gl_in[0].gl_Position.xyz;
  vec3 P1 = gl_in[1].gl_Position.xyz;
  vec3 P2 = gl_in[2].gl_Position.xyz;
  
  vec3 V0 = P0 - P1;
  vec3 V1 = P2 - P1;
  
  vec3 N = cross(V1, V0);
  N = normalize(N);
  
  // Center of the triangle
  vec3 P = (P0+P1+P2) / 3.0;
  
  mat4 mvp = ubo.projection;
  
  gl_Position = mvp * vec4(P, 1.0);
  vertex_color = vec3(1, 0, 0);
  EmitVertex();
  
  gl_Position = mvp * vec4(P + N * (normal_length), 1.0);
  vertex_color = vec3(1, 0, 0);
  EmitVertex();
  EndPrimitive();
}
