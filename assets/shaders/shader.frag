#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in vec3 fNormal;
layout(location = 1) in vec3 fPos;
layout(location = 2) in vec2 fTexCoord;
layout(location = 3) flat in int fTextureID;
layout(location = 4) in vec4 fViewPos;
layout(location = 5) in vec3 fAmbient;
layout(location = 6) in vec3 fAmbientLight;
layout(location = 7) in vec3 fLightDir;
layout(location = 8) in vec3 fDiffuseLight;
layout(location = 9) in vec3 fSpecular;
layout(location = 10) in vec3 fSpecularLight;
layout(location = 11) in float fShininess;
layout(location = 12) flat in int fMapsUsed;
layout(location = 13) in float fConstant;
layout(location = 14) in float fLinear;
layout(location = 15) in float fQuadratic;

layout(location = 16) in VS_OUT {
//    vec3 FragPos;
//    vec2 TexCoords;
    mat3 TBN;
} fs_in;

layout(binding = 1) uniform sampler2D texSampler[3];

float near = 0.1;
float far  = 100.0;
  
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
    vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient
    vec3 debugSpecular = vec3(0.0, 0.0, 1.0); // Debugging color for specular

//    if (texture(texSampler[0], fTexCoord).a < 0.5) {
//        discard;
//    }
    vec3 normal = fNormal;
    vec3 norm = normalize(normal);

    if (fMapsUsed >= 2) {
        normal = texture(texSampler[2], fTexCoord).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        norm = normal;
    }

    vec3 lightDir = normalize(-fLightDir);
    float distance = length(lightDir - fPos);
    float attenuation = 1.0 / (fConstant + fLinear * distance + fQuadratic * (distance * distance));

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * fDiffuseLight * texture(texSampler[0], fTexCoord).xyz;
    diffuse *= attenuation;

    vec3 ambient = fAmbientLight * texture(texSampler[0], fTexCoord).xyz;
    ambient *= attenuation;

    vec3 viewDir = normalize(fViewPos.xyz - fPos.xyz);
    //vec3 reflectDir = reflect(-fLightDir, norm);
    vec3 h = normalize(-fLightDir + viewDir);

    float specFactor = max(dot(norm, h), 0.0);
    vec3 specular = vec3(0.0);
    
    if (fMapsUsed >= 1) {
        float spec = pow(specFactor, fShininess);
        specular = (spec * fSpecularLight * texture(texSampler[1], fTexCoord).xyz) / distance;
    }

    specular *= attenuation;

    vec3 texture = texture(texSampler[0], fTexCoord).xyz;
    vec3 phong = (ambient + diffuse + specular) * texture;

    fColor = vec4(phong, 1.0f);
//    float depth = LinearizeDepth(gl_FragCoord.z) / far;
//    fColor = vec4(vec3(depth), 1.0);
}  