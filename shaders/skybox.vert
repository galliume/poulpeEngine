  #version 450 

struct UBO 
{
    mat4 model;
    mat4 view;
    mat4 proj;
}; 

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[6];
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 texCoord;
layout(location = 0) out vec3 fragTexCoord;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() { 
     //gl_Position = vec4(pos, 1.0);
    gl_Position = ubos[gl_InstanceIndex].proj * ubos[gl_InstanceIndex].view * vec4(pos, 1.0);
    fragTexCoord = colors[gl_VertexIndex];
}