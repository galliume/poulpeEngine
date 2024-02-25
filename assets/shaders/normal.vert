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
    //texture ID blank blank
    vec3 textureIDBB;
    mat4 view;
    vec4 viewPos;
    vec4 mapsUsed;
} pc;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in vec4 fidtidBB;
layout(location = 6) in vec3 vColor;

layout(location = 0) out VS_OUT {
    mat4 MPV;//matrice projection*view
    mat4 ITMPV;//inverse transpose matrice projection*view
    flat int fTextureID;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec4 fMapsUsed;
    vec4 fViewPos;
    mat3 TBN;
    vec4 fTangent;
    vec3 fBitangent;
    vec4 fShadowCoordAmbient;
    vec4 fShadowCoordSpot;
    //faceId texture ID blank blank
    vec4 ffidtidBB;
    vec3 fvColor;
    vec3 viewT;
    vec3 lightT;
    vec3 posT;
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

#define NR_POINT_LIGHTS 2
layout(binding = 3) buffer ObjectBuffer {
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

void CalculateTangentSpaceVL(vec3 position, vec3 normal, vec4 tangent, vec3 v, vec3 l, out vec3 vtan, out vec3 ltan);

//layout(binding = 2) uniform sampler2D texSampler2[4];

void main()
{
    gl_Position = ubos[gl_InstanceIndex].projection * pc.view * ubos[gl_InstanceIndex].model * vec4(pos, 1.0);

    vec3 norm = mat3(transpose(inverse(ubos[gl_InstanceIndex].model))) * normal;

    vec3 t = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(tangent.xyz, 0.0)));
    vec3 n = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(normal, 0.0)));
    vec3 b = normalize(vec3(ubos[gl_InstanceIndex].model * vec4(cross(n, t) * tangent.w, 0.0)));

    vs_out.TBN = transpose(mat3(t, b, n));//from tangent space to object space

    vs_out.fTangent = tangent;
    vs_out.fNormal = norm;
    vs_out.fBitangent = bitangent;
    vs_out.fPos = vec3(ubos[gl_InstanceIndex].model * vec4(pos, 1.0));
    vs_out.fTexCoord = texCoord;
    vs_out.fTextureID = int(pc.textureIDBB.x);//ID conversion should be ok
    vs_out.fMapsUsed = pc.mapsUsed;
    vs_out.fViewPos = pc.viewPos;
    vs_out.fShadowCoordAmbient = (biasMat * ambientLight.lightSpaceMatrix * ubos[gl_InstanceIndex].model) * vec4(pos, 1.0);
    vs_out.fShadowCoordSpot = (biasMat * spotLight.lightSpaceMatrix * ubos[gl_InstanceIndex].model) * vec4(pos, 1.0);
    vs_out.ffidtidBB = fidtidBB;
    vs_out.fvColor = vColor;

    vs_out.posT = vs_out.TBN * vs_out.fPos;

//    if (vs_out.fMapsUsed.x > 0.9 && vs_out.fMapsUsed.x < 1.1) {
//      vec3 n = normalize(vs_out.fNormal);
//      vec2 tmpM = texture(texSampler2[1], texCoord).xy;
//      vs_out.fNormal = vec3(tmpM, sqrt(1.0 - tmpM.x * tmpM.x - tmpM.y * tmpM.y));
//    }
//
    CalculateTangentSpaceVL(vs_out.fPos, vs_out.fNormal, vs_out.fTangent, vs_out.fViewPos.xyz, ambientLight.position, vs_out.viewT, vs_out.lightT);
}

void CalculateTangentSpaceVL(vec3 position, vec3 normal, vec4 tangent, vec3 cameraPos, vec3 lightPos, out vec3 vtan, out vec3 ltan)
{ 
  vec3 bitangent = cross(normal, tangent.xyz) * tangent.w;
  vec3 v = cameraPos - position;
  vec3 l = lightPos - position;
  vtan = vec3(dot(tangent.xyz, v), dot(bitangent, v), dot(normal, v));
  ltan = vec3(dot(tangent.xyz, l), dot(bitangent, l), dot(normal, l));
}