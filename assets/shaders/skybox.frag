 #version 450 

layout(location = 0) out vec4 outputColor;
layout(location = 0) in vec3 fragTexCoord;

layout(binding = 1) uniform samplerCubeArray texSampler;

void main() { 
    outputColor = textureLod(texSampler, vec4(fragTexCoord, 1), 1.0f); 
    //outputColor = vec4(fragTexCoord, 1.0);
}