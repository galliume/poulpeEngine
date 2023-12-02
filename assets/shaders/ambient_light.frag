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
layout(location = 6) in vec3 fDiffuse;
layout(location = 7) in vec3 fSpecular;
layout(location = 8) in float fShininess;

layout(binding = 1) uniform sampler2D texSampler[];

void main()
{
    vec3 lightPos = vec3(0.5f, 4.5f, -3.0f);
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

    vec3 norm = normalize(fNormal);
    vec3 lightDir = normalize(lightPos - fPos);

    float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
    vec3 diffuse = (diff * fDiffuse) * lightColor;

    vec3 ambient = fAmbient * lightColor;

    vec3 viewDir = normalize(fViewPos.xyz - fPos.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
    vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient

    vec3 texture = texture(texSampler[fTextureID], fTexCoord).xyz;
    vec3 phong = (ambient + diffuse) * texture;
    fColor = vec4(phong, 1.0f);
}  