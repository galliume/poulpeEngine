#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(location = 0) in VS_OUT {
    flat int fTextureID;
    float fConstant;
    float fLinear;
    float fQuadratic;
    vec2 fTexCoord;
    vec2 fTexSize;
    vec3 fNormal;
    vec3 fPos;
    vec3 fMapsUsed;
    vec4 fViewPos;
    mat3 TBN;
} fs_in;

struct Light {
    vec3 color;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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

layout(binding = 1) uniform sampler2D texSampler[3];

layout(binding = 2) buffer ObjectBuffer {
    Light ambientLight;
    Material material;
};

float near = 0.1;
float far  = 100.0;
  
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}


//vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float distance, float attenuation);

void main()
{
    vec3 debugDiffuse = vec3(1.0, 0.0, 0.0); // Debugging color for diffuse
    vec3 debugAmbient = vec3(0.0, 1.0, 0.0); // Debugging color for ambient
    vec3 debugSpecular = vec3(0.0, 0.0, 1.0); // Debugging color for specular

//    if (texture(texSampler[0], fs_in.fTexCoord).a < 0.5) {
//        discard;
//    }

    vec3 normal = normalize(fs_in.fNormal);

    if (fs_in.fMapsUsed.x > 0.0) {
      vec3 nm = texture(texSampler[2], fs_in.fTexCoord).xyz * 2.0 - vec3(1.0);
      nm = fs_in.TBN * nm;
      normal = vec3(normalize(nm));
    }

    vec3 lightDir = normalize(ambientLight.direction - fs_in.fPos);
    float distance = length(lightDir - fs_in.fPos);
    float attenuation = 1.0 / (fs_in.fConstant + fs_in.fLinear * distance + fs_in.fQuadratic * (distance * distance));
    
    vec3 ambient = ambientLight.ambient * material.ambient * ambientLight.color;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * ambientLight.color * diff * ambientLight.diffuse * texture(texSampler[0], fs_in.fTexCoord).xyz;

    vec3 viewDir = normalize(fs_in.fViewPos.xyz - fs_in.fPos.xyz);
    vec3 h = normalize(lightDir + viewDir);
    vec3 specular = vec3(0.0);

    if (fs_in.fMapsUsed.y > 0.0) {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, lightDir) > 0.0);
      specular = ambientLight.color * (spec * material.specular * texture(texSampler[1], fs_in.fTexCoord).xyz) / distance;
    } else {
      float spec = pow(clamp(dot(normal, h), 0.0, 1.0), material.shiIorDiss.x) * float(dot(normal, h) > 0.0) * float(dot(normal, lightDir) > 0.0);
      specular = ambientLight.color * (ambientLight.specular * spec * material.specular) / distance;
    }

    float gamma = 2.2;

   //diffuse = pow(diffuse, vec3(gamma));
   //ambient = pow(ambient, vec3(gamma));
   //specular = pow(specular, vec3(gamma));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    fColor = vec4((ambient + diffuse + specular), 1.0f) * texture(texSampler[0], fs_in.fTexCoord);
    //float depth = LinearizeDepth(gl_FragCoord.z) / far;

//    float alpha = material.shiIorDiss.x / 1000;
//    fColor = vec4(0.5,0.0,0.0, alpha);
}