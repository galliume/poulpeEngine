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
  vec4 view_position;
  vec4 total_position;
} pc;

layout(location = 0) in vec3 position;

void main()
{
  gl_Position = pc.view * ubo.model * vec4(position, 1.0f);
}