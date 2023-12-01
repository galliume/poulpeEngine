#version 450 
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in vec3 fTexCoord;
layout(location = 1) in vec4 cameraPos;
layout(location = 2) in vec4 fModelPos;

layout(binding = 1) uniform samplerCube texSampler[];

void main() 
{ 
    fColor = texture(texSampler[0], fTexCoord);
	
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

	vec3 ambient = lightColor * lightColor;
    fColor = vec4(ambient, 1.0f) * fColor;
}
