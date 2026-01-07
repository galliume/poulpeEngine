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

layout(location = 0) in vec4 tangent;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 original_pos;
layout(location = 5) in vec2 texture_coord;
layout(location = 6) in ivec4 bone_ids;
layout(location = 7) in vec4 bone_weights;
layout(location = 8) in float total_weight;

void main()
{
  gl_Position = pc.view * ubo.model * vec4(position, 1.0f);
}