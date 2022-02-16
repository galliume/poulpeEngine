#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outputColor;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in int index;

layout(binding = 1) uniform sampler2D texSampler[];

layout(push_constant) uniform PER_OBJECT
{
	int imgIdx;
}pc;

void main() {    
    outputColor = texture(texSampler[index], fragTexCoord);    
}