#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(binding = 1) uniform sampler2D texSampler[3];

layout (location = 0) in vec2 inUV;


float near = 0.1;
float far  = 100.0;
  
float LinearizeDepth(float depth)
{
    return (2.0 * near) / (far + near - depth * (far - near));
}

void main()
{
	//float depthValue = texture(texSampler[0], inUV).r;
    //fColor = vec4(vec3(1.0-LinearizeDepth(depthValue)), 1.0);
	fColor = vec4(1.0, 0.0, 0.0, 1.0);
}