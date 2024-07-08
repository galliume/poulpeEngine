#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_UBOS 1

struct UBO
{
    mat4 model;
    mat4 projection;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(push_constant) uniform constants
{
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
} pc;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 fidtidBB;
layout(location = 5) in vec3 vColor;

struct Light {
    vec3 color;
    vec3 direction;
    vec3 position;
    //ambiance diffuse specular
    vec3 ads;
    //constant, linear, quadratiq
    vec3 clq;
    //cutOff, outerCutoff, Blank
    vec3 coB;
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
};

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 transmittance;
    vec3 emission;
    //shininess, ior, diss
    vec3 shiIorDiss;
};

#define NR_POINT_LIGHTS 2
layout(set = 0, binding = 1) buffer ObjectBuffer {
    Light ambientLight;
    Light pointLights[NR_POINT_LIGHTS];
    Light spotLight;
    Material material;
};

void main()
{
    gl_Position = pointLights[0].lightSpaceMatrix * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

    //outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    //gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
} 