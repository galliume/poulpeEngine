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
layout(location = 7) in vec3 fAmbientLighPos;
layout(location = 8) in vec3 fAmbientLightColor;
layout(location = 9) in vec3 fDiffuse;
layout(location = 10) in vec3 fDiffuseLight;
layout(location = 11) in vec3 fSpecular;
layout(location = 12) in vec3 fSpecularLight;
layout(location = 13) in float fShininess;

layout(binding = 1) uniform sampler2D texSampler[];

void main()
{
    vec3 norm = normalize(fNormal);
    vec3 lightDir = fAmbientLighPos - fPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    distance = distance * distance;

    float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
    vec3 diffuse = fAmbientLightColor * (diff * fDiffuse * fDiffuseLight);

    vec3 ambient = fAmbientLightColor * fAmbient * fAmbientLight;

    vec3 viewDir = normalize(fViewPos.xyz - fPos.xyz);
    //vec3 reflectDir = reflect(-lightDir, norm);
    vec3 h = normalize(-lightDir + viewDir);

    float specFactor = clamp(dot(norm, h), 0.0, 1.0);
    vec3 specular = vec3(0.0);

    if (specFactor > 0.0) {
        float spec = pow(specFactor, fShininess) * float(dot(norm, -lightDir) > 0.0);
        vec3 specular = fAmbientLightColor * (fSpecular * spec * fSpecularLight) / distance;
    }

    vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
    vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient
    vec3 debugSpecular = vec3(0.0, 0.0, 1.0); // Debugging color for specular

    vec3 texture = texture(texSampler[fTextureID], fTexCoord).xyz;
    vec3 phong = (ambient + diffuse) * texture + specular;

    fColor = vec4(phong, 1.0f);
}  