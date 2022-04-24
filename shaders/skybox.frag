 #version 450 

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec3 fragTexCoord;

layout(binding = 1) uniform samplerCubeArray texSampler;

struct UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
    int index;
}; 

layout(set = 0, binding = 0) uniform CubeUniformBufferObject {
   UBO ubo;
};

void main() { 
    outputColor = textureLod(texSampler, vec4(fragTexCoord, 1), 1.0f); 
    //outputColor = vec4(fragTexCoord, 1.0);
}