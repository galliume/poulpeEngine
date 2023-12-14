#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

void main()
{
	gl_FragDepth = gl_FragCoord.z;
	fColor = vec4(1.0, 0.0, 0.0, 1.0);
}