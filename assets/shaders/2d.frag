#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

layout(push_constant) uniform constants
{
    int textureID;
}PC;

void main() {
    outColor = texture(texSampler[PC.textureID], fragTexCoord);
    //outColor = vec4(fragColor, 1.0);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}