#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define NR_POINT_LIGHTS 2

struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
  UBO ubo;
};

layout(push_constant) uniform constants
{
  mat4 view;
  vec3 view_position;
  vec4 total_position;
} pc;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out GEOM_VAR {
  vec3 norm;
  mat4 view;
} geom_var;

void main()
{
  geom_var.norm = normal;
  geom_var.view = pc.view;
  
  gl_Position = pc.view * ubo.model * vec4(position, 1.0f);
} 