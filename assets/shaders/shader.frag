#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    flat int fTextureID;
    float fShininess;
    float fConstant;
    float fLinear;
    float fQuadratic;
    vec2 fTexCoord;
    vec3 fNormal;
    vec3 fPos;
    vec3 fMapsUsed;
    vec3 fAmbient;
    vec3 fAmbientLight;
    vec3 fAmbientLightColor;
    vec3 fLightDir;
    vec3 fDiffuseLight;
    vec3 fSpecular;
    vec3 fSpecularLight;
    vec4 fViewPos;
    mat3 TBN;
} fs_in;

struct NormalMap {
    vec3 normal;
};

layout(binding = 1) uniform sampler2D texSampler[3];

layout(binding = 2) readonly buffer ObjectBuffer{
    NormalMap normalMap[];
} normalMapBuffer;

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

    if (texture(texSampler[0], fs_in.fTexCoord).a < 0.5) {
        discard;
    }
    vec3 normal = fs_in.fNormal;
    vec3 norm = normalize(normal);

    if (fs_in.fMapsUsed.x > 0.0) {
      normal = texture(texSampler[2], fs_in.fTexCoord).rgb;
      normal = normalize(normal * 2.0 - 1.0);
      norm = normal;
    }

    vec3 lightDir = normalize(fs_in.fLightDir - fs_in.fPos);
    float distance = length(lightDir - fs_in.fPos);
    float attenuation = 1.0 / (fs_in.fConstant + fs_in.fLinear * distance + fs_in.fQuadratic * (distance * distance));
    
    vec3 ambient = fs_in.fAmbientLight * fs_in.fAmbient * fs_in.fAmbientLightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = fs_in.fAmbientLightColor * diff * fs_in.fDiffuseLight * texture(texSampler[0], fs_in.fTexCoord).xyz;

    vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 h = normalize(-lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.0) {
      float spec = pow(clamp(dot(norm, h), 0.0, 1.0), fs_in.fShininess) * float(dot(norm, lightDir) > 0.0);
      specular = fs_in.fAmbientLightColor * (spec * fs_in.fSpecularLight * texture(texSampler[1], fs_in.fTexCoord).xyz) / distance;
    } else {
      float spec = pow(clamp(dot(norm, h), 0.0, 1.0), fs_in.fShininess) * float(dot(norm, h) > 0.0) * float(dot(norm, lightDir) > 0.0);
      specular = fs_in.fAmbientLightColor * (fs_in.fSpecular * spec * fs_in.fSpecularLight) / distance;
    }

    float gamma = 2.2;

   //diffuse = pow(diffuse, vec3(gamma));
   //ambient = pow(ambient, vec3(gamma));
   //specular = pow(specular, vec3(gamma));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    fColor = vec4((ambient + diffuse + specular), 1.0f) * texture(texSampler[0], fs_in.fTexCoord);
//    float depth = LinearizeDepth(gl_FragCoord.z) / far;
//    fColor = vec4(vec3(depth), 1.0);
}  