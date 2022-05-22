#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

struct UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[341];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec4 fogDensity;

float fogDepthMax = 20.0;
float fogDepthMin = 10.0;

layout(push_constant) uniform constants
{
    vec3 cameraPos;
} PushConstants;

void main() {
    
    vec4 depthVert = vec4(PushConstants.cameraPos, 1.0) * vec4(pos, 1.0);
    float depth = abs(depthVert.z / depthVert.w);
    fogDensity.x = 1.0 - clamp((fogDepthMax - depth) / (fogDepthMin - fogDepthMax), 0.0, 1.0);

    gl_Position = ubos[gl_InstanceIndex].proj * ubos[gl_InstanceIndex].view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);
    fragTexCoord = texCoord;
    fragColor = color;
}
