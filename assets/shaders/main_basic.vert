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
  vec4 fAmbientLightSpace;
  vec4 fShadowCoordSpot;
  //faceId texture ID blank blank
  vec4 ffidtidBB;
  vec3 fvColor;
  vec3 TangentLightPos;
  vec3 TangentViewPos;
  vec3 TangentFragPos;
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
  vec3 shi_ior_diss;
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
  0.5, 0.5, 0.0, 1.0);

void main()
{
  mat3 normalMatrix = transpose(inverse(mat3(ubos[gl_InstanceIndex].model)));

  vec3 t = normalize(normalMatrix * tangent.xyz);
  vec3 n = normalize(normalMatrix * normal);
  t = normalize(t - dot(t, n) * n);
  vec3 b = cross(n, t);
  mat3 TBN = transpose(mat3(t, b, n));

  vs_out.fNormal = normalMatrix * normal;
  vs_out.fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
  vs_out.fNormal = (ubos[gl_InstanceIndex].model * vec4(normal, 0.0)).xyz;
  vs_out.fTexCoord = texCoord;
  vs_out.fViewPos = pc.viewPos;
  vs_out.fAmbientLightSpace = (biasMat * ambientLight.lightSpaceMatrix * ubos[gl_InstanceIndex].model) * vec4(pos, 1.0);
  vs_out.fShadowCoordSpot = (biasMat * spotLight.lightSpaceMatrix) * vec4(vs_out.fPos, 1.0);
  vs_out.ffidtidBB = fidtidBB;
  vs_out.fvColor = vColor;
  vs_out.fTextureID = int(fidtidBB.y);
  vs_out.TangentLightPos = TBN * ambientLight.position;
  vs_out.TangentViewPos  = TBN * vs_out.fViewPos.xyz;
  vs_out.TangentFragPos  = TBN * vs_out.fPos;

  gl_Position = ubos[gl_InstanceIndex].projection * pc.view * vec4(vs_out.fPos, 1.0);
} 