#pragma once

namespace Poulpe
{
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 projection;
        alignas(4) float constant{ 1.0 };
        alignas(4) float linear{ 0.09 };
        alignas(4) float quadratic{ 0.032 };
        alignas(8) glm::vec2 texSize;
    };

    struct CubeUniformBufferObject : UniformBufferObject
    {
        int index;
    };

    struct Light
    {
        alignas(16) glm::vec3 color;
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 position;
        //ambiance diffuse specular
        alignas(16) glm::vec3 ads;
        //constant, linear, quadratiq
        alignas(16) glm::vec3 clq;
        //cutOff, outerCutoff Blank
        alignas(16) glm::vec3 coB{ 1.0, 0.0, 0.0 };
    };

    struct Material
    {
        alignas(16) glm::vec3 ambient;
        alignas(16) glm::vec3 diffuse;
        alignas(16) glm::vec3 specular;
        alignas(16) glm::vec3 transmittance;
        alignas(16) glm::vec3 emission;
        //shininess, ior, diss
        alignas(16) glm::vec3 shiIorDiss;
    };

    struct ObjectBuffer
    {
        Light ambientLight;
        std::array<Light, 2> pointLights;
        Light spotLight;
        Material material;
    };
}
