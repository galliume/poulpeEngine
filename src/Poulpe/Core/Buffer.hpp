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
    };

    struct CubeUniformBufferObject : UniformBufferObject
    {
        int index;
    };
}
