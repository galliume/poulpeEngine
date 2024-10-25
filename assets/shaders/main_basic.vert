#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_UBOS 1
#define NR_POINT_LIGHTS 2

struct UBO
{
    mat4 model;
    mat4 projection;
};

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
   UBO ubos[MAX_UBOS];
};

layout(push_constant) uniform constants
{
    //texture ID blank blank
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
    vec4 totalPosition;
} pc;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 fidtidBB;
layout(location = 5) in vec3 vColor;

layout(location = 0) out VS_OUT {
    flat int fTextureID;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec4 fViewPos;
    mat3 TBN;
    vec4 fAmbientLightSpace;
    vec4 fShadowCoordSpot;
    //faceId texture ID blank blank
    vec4 ffidtidBB;
    vec3 fvColor;
} vs_out;


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

layout(set = 0, binding = 2) readonly buffer ObjectBuffer {
    Light ambientLight;
    Light pointLights[NR_POINT_LIGHTS];
    Light spotLight;
    Material material;
};

const mat4 biasMat = mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 );

void main()
{
    vec3 t = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(tangent.xyz, 0.0)));
    vec3 n = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(normal, 0.0)));
    vec3 b = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(cross(n, t) * tangent.w, 0.0)));
    mat3 TBN = transpose(mat3(t, b, n));
    vs_out.TBN = TBN;

    //vs_out.fNormal = normal;
    vs_out.fNormal = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;
    vs_out.fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    vs_out.fTexCoord = texCoord;
    vs_out.fTextureID = int(pc.textureIDBB.x);//ID conversion should be ok
    vs_out.fViewPos = pc.viewPos;
    vs_out.fAmbientLightSpace = ambientLight.lightSpaceMatrix * vec4(vs_out.fPos, 1.0);
    vs_out.fShadowCoordSpot = (biasMat * spotLight.lightSpaceMatrix) * vec4(vs_out.fPos, 1.0);
    vs_out.ffidtidBB = fidtidBB;
    vs_out.fvColor = vColor;

    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * vec4(vs_out.fPos, 1.0) * pc.totalPosition;
} 