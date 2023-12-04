#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    vec3 fNormal;
    vec3 fPos;
    vec2 fTexCoord;
    flat int fTextureID;
    vec4 fViewPos;
    vec3 fAmbient;
    vec3 fAmbientLight;
    vec3 fLightDir;
    vec3 fDiffuseLight;
    vec3 fSpecular;
    vec3 fSpecularLight;
    float fShininess;
    float fConstant;
    float fLinear;
    float fQuadratic;
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
    vec3 normal = fs_in.fNormal;
    vec3 norm = normalize(normal);

    normal = texture(texSampler[2], fs_in.fTexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    norm = normal;

    vec3 lightDir = normalize(fs_in.fLightDir - fs_in.fPos);
    float distance = length(lightDir - fs_in.fPos);
    float attenuation = 1.0 / (fs_in.fConstant + fs_in.fLinear * distance + fs_in.fQuadratic * (distance * distance));
    
    vec3 ambient = fs_in.fAmbientLight * fs_in.fAmbient;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = fs_in.fAmbient * (diff * fs_in.fDiffuseLight * texture(texSampler[0], fs_in.fTexCoord).xyz);

    vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 h = normalize(-lightDir + viewDir);
    
    float spec = pow(clamp(dot(norm, h), 0.0, 1.0), fs_in.fShininess) * float(dot(norm, lightDir) > 0.0);
    vec3 specular = fs_in.fAmbient * ((spec * fs_in.fSpecularLight * texture(texSampler[1], fs_in.fTexCoord).xyz) / distance);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 texture = texture(texSampler[0], fs_in.fTexCoord).xyz;
    vec3 phong = (ambient + diffuse) * texture;

    fColor = vec4(phong, 1.0f);
//    float depth = LinearizeDepth(gl_FragCoord.z) / far;
//    fColor = vec4(vec3(depth), 1.0);
}  