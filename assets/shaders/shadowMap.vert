#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO
{
  mat4 model;
  mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
  UBO ubo;
};

layout(location = 0) in vec3 position;

layout(push_constant) uniform constants
{
  mat4 view;
} pc;

void main()
{
  gl_Position = ubo.projection * pc.view * (ubo.model * vec4(position, 1.0));
} 