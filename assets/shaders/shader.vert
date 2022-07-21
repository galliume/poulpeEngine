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
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragModelPos;

layout(push_constant) uniform constants
{
    int textureID;
    vec4 cameraPos;
    float ambiantLight;
    float fogDensity;
    vec3 fogColor;
    vec4 lightPos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} PC;

void main() {
    
    gl_Position = ubos[gl_InstanceIndex].proj * ubos[gl_InstanceIndex].view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);
    fragTexCoord = texCoord;
    
    //@todo move it to application
    fragNormal =  mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;

    fragModelPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
}
