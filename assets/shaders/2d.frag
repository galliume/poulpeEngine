#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

layout(push_constant) uniform constants
{
    //texture ID blank blank
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
    vec3 mapsUsed;
} pc;

void main() {
	int id = int(pc.textureIDBB.x);
    outColor = texture(texSampler[id], fragTexCoord);
    //outColor = vec4(fragColor, 1.0);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}